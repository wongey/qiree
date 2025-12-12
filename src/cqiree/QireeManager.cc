//----------------------------------*-C++-*----------------------------------//
// Copyright 2025 UT-Battelle, LLC, and other QIR-EE developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//---------------------------------------------------------------------------//
//! \file cqiree/QireeManager.cc
//---------------------------------------------------------------------------//
#include "QireeManager.hh"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "qiree_config.h"

#include "qiree/Assert.hh"
#include "qiree/Executor.hh"
#include "qiree/Module.hh"
#include "qiree/QuantumInterface.hh"
#include "qiree/RecordedResult.hh"
#include "qiree/ResultDistribution.hh"
#include "qiree/SingleResultRuntime.hh"
#include "qirqsim/QsimQuantum.hh"
#include "qirqsim/QsimRuntime.hh"

#define CQIREE_FAIL(CODE, MESSAGE)                         \
    do                                                     \
    {                                                      \
        std::cerr << "qiree failure: " << MESSAGE << '\n'; \
        return ReturnCode::CODE;                           \
    } while (0)

namespace qiree
{
//---------------------------------------------------------------------------//
QireeManager::QireeManager() throw()
{
    // Seed random number generator for unique temp file names
    static bool seeded = false;
    if (!seeded)
    {
        std::srand(std::time(nullptr));
        seeded = true;
    }
}
QireeManager::~QireeManager() throw() = default;

//---------------------------------------------------------------------------//
QireeManager::ReturnCode
QireeManager::load_module(std::string_view data_contents) throw()
{
    try
    {
        // Convert string_view to string for Module constructor
        std::string content{data_contents};
        module_ = Module::from_bytes(content);
    }
    catch (std::exception const& e)
    {
        CQIREE_FAIL(fail_load, e.what());
    }
    return ReturnCode::success;
}

//---------------------------------------------------------------------------//
QireeManager::ReturnCode QireeManager::load_module(std::string filename) throw()
{
    try
    {
        module_ = std::make_unique<Module>(filename);
        QIREE_ENSURE(*module_);
    }
    catch (std::exception const& e)
    {
        std::cerr << "qiree failure: " << e.what() << '\n';
        CQIREE_FAIL(fail_load, e.what());
    }

    return ReturnCode::success;
}

//---------------------------------------------------------------------------//
QireeManager::ReturnCode QireeManager::num_quantum_reg(int& result) const
    throw()
{
    if (!module_)
    {
        CQIREE_FAIL(not_ready, "cannot query module before module load");
    }
    if (execute_)
    {
        CQIREE_FAIL(not_ready, "cannot query module after creating executor");
    }

    auto attrs = module_->load_entry_point_attrs();
    result = attrs.required_num_qubits;
    return ReturnCode::success;
}

//---------------------------------------------------------------------------//
QireeManager::ReturnCode QireeManager::num_classical_reg(int& result) const
    throw()
{
    if (!module_)
    {
        CQIREE_FAIL(not_ready, "cannot query module before module load");
    }
    if (execute_)
    {
        CQIREE_FAIL(not_ready, "cannot query module after creating executor");
    }

    auto attrs = module_->load_entry_point_attrs();
    result = attrs.required_num_results;
    return ReturnCode::success;
}

//---------------------------------------------------------------------------//
QireeManager::ReturnCode
QireeManager::max_result_items(int num_shots, std::size_t& result) const
    throw()
{
    if (!module_)
    {
        CQIREE_FAIL(not_ready,
                    "cannot determine result size before module load");
    }

    if (num_shots <= 0)
    {
        CQIREE_FAIL(invalid_input, "num_shots was nonpositive");
    }

    // Maximum result
    auto attrs = module_->load_entry_point_attrs();
    auto num_registers = attrs.required_num_results;
    result = std::min<std::size_t>(1 << num_registers, num_shots);

    // Add one for the "count" record at the beginning
    result++;

    return ReturnCode::success;
}

//---------------------------------------------------------------------------//
QireeManager::ReturnCode
QireeManager::setup_executor(std::string_view backend,
                             std::string_view config_json) throw()
{
    if (!module_)
    {
        CQIREE_FAIL(not_ready, "cannot create executor before module load");
    }
    if (execute_)
    {
        CQIREE_FAIL(not_ready, "cannot create executor again");
    }

    try
    {
        if (backend == "qsim")
        {
#if QIREE_USE_QSIM
            // qsim backend doesn't support config_json
            if (!config_json.empty())
            {
                QIREE_NOT_IMPLEMENTED(
                    "CQiree JSON configuration input for qsim backend");
            }

            // Create runtime interface: give runtime a pointer to quantum
            // (lifetime of the reference is guaranteed by our shared pointer
            // copy)
            constexpr unsigned long int seed = 0;
            auto quantum = std::make_shared<QsimQuantum>(std::cout, seed);
            runtime_ = std::make_shared<QsimRuntime>(std::cout, *quantum);
            quantum_ = std::move(quantum);
#else
            QIREE_NOT_CONFIGURED("QSim");
#endif
        }
        else if (backend == "xacc")
        {
#if QIREE_USE_XACC
            // XACC backend requires qir-xacc command-line tool
            // Parse config_json for accelerator and shots
            std::string accelerator = "default";
            int shots = 1024;

            if (!config_json.empty())
            {
                try
                {
                    // Simple JSON parsing for accelerator and shots
                    // Extract accelerator
                    std::string json_str{config_json};
                    auto accel_pos = json_str.find("\"accelerator\"");
                    if (accel_pos != std::string::npos)
                    {
                        auto value_start = json_str.find("\"", accel_pos + 14);
                        if (value_start != std::string::npos)
                        {
                            auto value_end
                                = json_str.find("\"", value_start + 1);
                            if (value_end != std::string::npos)
                            {
                                accelerator = json_str.substr(
                                    value_start + 1,
                                    value_end - value_start - 1);
                            }
                        }
                    }

                    // Extract shots
                    auto shots_pos = json_str.find("\"shots\"");
                    if (shots_pos != std::string::npos)
                    {
                        auto value_start = shots_pos + 7;
                        while (value_start < json_str.size()
                               && (json_str[value_start] == ':'
                                   || std::isspace(json_str[value_start])))
                        {
                            value_start++;
                        }

                        auto value_end = value_start;
                        while (value_end < json_str.size()
                               && std::isdigit(json_str[value_end]))
                        {
                            value_end++;
                        }

                        if (value_end > value_start)
                        {
                            shots = std::stoi(json_str.substr(
                                value_start, value_end - value_start));
                        }
                    }
                }
                catch (std::exception const& e)
                {
                    CQIREE_FAIL(invalid_input,
                                "failed to parse config_json: " << e.what());
                }
            }

            // Write module to temporary .ll file with unique name
            // Use time + random number to avoid collisions in parallel
            // execution
            std::string temp_ll_file
                = "/tmp/qiree_module_" + std::to_string(std::time(nullptr))
                  + "_" + std::to_string(std::rand()) + ".ll";
            std::string temp_output_file = temp_ll_file + ".out";

            try
            {
                module_->write_to_file(temp_ll_file);
            }
            catch (std::exception const& e)
            {
                CQIREE_FAIL(fail_load,
                            "failed to write module to file: " << e.what());
            }

            // Construct and execute qir-xacc command, redirecting output to
            // file
            std::string cmd = "qir-xacc -a " + accelerator + " -s "
                              + std::to_string(shots) + " " + temp_ll_file
                              + " > " + temp_output_file + " 2>&1";

            int ret = std::system(cmd.c_str());

            // Clean up temporary LL file
            std::filesystem::remove(temp_ll_file);

            if (ret != 0)
            {
                // Try to read error output before failing
                std::ifstream error_file(temp_output_file);
                if (error_file.is_open())
                {
                    std::string error_msg(
                        (std::istreambuf_iterator<char>(error_file)),
                        std::istreambuf_iterator<char>());
                    std::filesystem::remove(temp_output_file);
                    CQIREE_FAIL(fail_execute,
                                "qir-xacc command failed: " << error_msg);
                }
                std::filesystem::remove(temp_output_file);
                CQIREE_FAIL(fail_execute,
                            "qir-xacc command failed with return code " << ret);
            }

            // Read and parse the output to extract Measurements
            std::ifstream output_file(temp_output_file);
            if (!output_file.is_open())
            {
                std::filesystem::remove(temp_output_file);
                CQIREE_FAIL(fail_execute, "failed to read qir-xacc output");
            }

            std::string output_content(
                (std::istreambuf_iterator<char>(output_file)),
                std::istreambuf_iterator<char>());
            std::filesystem::remove(temp_output_file);

            // Parse the JSON to extract Measurements
            // Find the "Measurements" section in the JSON
            auto measurements_pos = output_content.find("\"Measurements\"");
            if (measurements_pos == std::string::npos)
            {
                CQIREE_FAIL(fail_execute,
                            "could not find Measurements in XACC output");
            }

            // Find the opening brace of the Measurements object
            auto open_brace = output_content.find("{", measurements_pos);
            if (open_brace == std::string::npos)
            {
                CQIREE_FAIL(fail_execute,
                            "malformed Measurements section in XACC output");
            }

            // Find the closing brace (handle nested braces)
            int brace_count = 1;
            std::size_t close_brace = open_brace + 1;
            while (close_brace < output_content.size() && brace_count > 0)
            {
                if (output_content[close_brace] == '{')
                    brace_count++;
                else if (output_content[close_brace] == '}')
                    brace_count--;
                close_brace++;
            }

            if (brace_count != 0)
            {
                CQIREE_FAIL(fail_execute,
                            "malformed Measurements section in XACC output");
            }

            // Extract the measurements content (without braces)
            std::string measurements_content = output_content.substr(
                open_brace + 1, close_brace - open_brace - 2);

            // Parse measurement entries: "bitstring": count
            xacc_measurements_.clear();
            std::size_t pos = 0;
            while (pos < measurements_content.size())
            {
                // Find opening quote of bitstring
                auto quote1 = measurements_content.find("\"", pos);
                if (quote1 == std::string::npos)
                    break;

                // Find closing quote of bitstring
                auto quote2 = measurements_content.find("\"", quote1 + 1);
                if (quote2 == std::string::npos)
                    break;

                std::string bitstring = measurements_content.substr(
                    quote1 + 1, quote2 - quote1 - 1);

                // Find colon
                auto colon = measurements_content.find(":", quote2);
                if (colon == std::string::npos)
                    break;

                // Extract count value
                auto value_start = colon + 1;
                while (value_start < measurements_content.size()
                       && std::isspace(measurements_content[value_start]))
                {
                    value_start++;
                }

                auto value_end = value_start;
                while (value_end < measurements_content.size()
                       && std::isdigit(measurements_content[value_end]))
                {
                    value_end++;
                }

                if (value_end > value_start)
                {
                    int count = std::stoi(measurements_content.substr(
                        value_start, value_end - value_start));
                    xacc_measurements_[bitstring] = count;
                }

                pos = value_end;
            }

            if (xacc_measurements_.empty())
            {
                CQIREE_FAIL(fail_execute,
                            "no measurements extracted from XACC output");
            }

            // Store the number of shots for later use in execute()
            xacc_shots_ = shots;
            using_xacc_ = true;

            // Don't create runtime/quantum for XACC - we'll use stored
            // measurements
#else
            QIREE_NOT_CONFIGURED("XACC");
#endif
        }
        else
        {
            QIREE_VALIDATE(false,
                           << "unknown backend name '" << backend << "'");
        }
    }
    catch (std::exception const& e)
    {
        CQIREE_FAIL(fail_load,
                    "error while creating quantum runtimes: " << e.what());
    }

    // Create executor only for non-XACC backends
    if (!using_xacc_)
    {
        try
        {
            // Create executor with the module, quantum and runtime interfaces
            QIREE_ASSERT(module_ && *module_);
            execute_ = std::make_unique<Executor>(std::move(*module_));
        }
        catch (std::exception const& e)
        {
            CQIREE_FAIL(fail_load,
                        "error while creating executor: " << e.what());
        }
    }

    return ReturnCode::success;
}

//---------------------------------------------------------------------------//
QireeManager::ReturnCode QireeManager::execute(int num_shots) throw()
{
    if (!using_xacc_ && !execute_)
    {
        CQIREE_FAIL(not_ready, "setup_executor was not created");
    }

    if (num_shots <= 0)
    {
        CQIREE_FAIL(invalid_input, "num_shots was nonpositive");
    }

    try
    {
        result_ = std::make_unique<ResultDistribution>();

        // Check if XACC measurements are available (XACC backend path)
        if (!xacc_measurements_.empty())
        {
            // Use pre-computed XACC measurements
            for (auto const& [bitstring, count] : xacc_measurements_)
            {
                // Convert bitstring to vector<bool>
                std::vector<bool> bits;
                bits.reserve(bitstring.size());
                for (char c : bitstring)
                {
                    bits.push_back(c == '1');
                }

                // Create RecordedResult and accumulate count times
                RecordedResult recorded_result(std::move(bits));
                for (int i = 0; i < count; ++i)
                {
                    result_->accumulate(recorded_result);
                }
            }
        }
        else
        {
            // Normal execution path (qsim or other backends)
            QIREE_ASSERT(runtime_ && quantum_);

            for (auto i = 0; i < num_shots; ++i)
            {
                (*execute_)(*quantum_, *runtime_);
                result_->accumulate(runtime_->result());
            }
        }
    }
    catch (std::exception const& e)
    {
        CQIREE_FAIL(fail_execute, e.what());
    }
    return ReturnCode::success;
}
//---------------------------------------------------------------------------//
QireeManager::ReturnCode
QireeManager::save_result_items(ResultRecord* encoded,
                                std::size_t max_count) throw()
{
    if (!result_)
    {
        CQIREE_FAIL(not_ready, "execute has not been called");
    }

    if (max_count < result_->size() + 1)
    {
        CQIREE_FAIL(fail_load,
                    "insufficient capacity " << max_count
                                             << " for result items: need "
                                             << result_->size() + 1);
    }

    try
    {
        // Save number of records
        *encoded++ = {0, result_->size()};
        for (auto&& [bitstring, count] : *result_)
        {
            std::uint64_t bitint{0};
            encode_bit_string(bitstring, bitint);
            *encoded++ = {bitint, count};
        }
    }
    catch (std::exception const& e)
    {
        CQIREE_FAIL(fail_execute, e.what());
    }
    return ReturnCode::success;
}
//---------------------------------------------------------------------------//
}  // namespace qiree
