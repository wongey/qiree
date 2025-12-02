//----------------------------------*-C++-*----------------------------------//
// Copyright 2024 UT-Battelle, LLC, and other QIR-EE developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//---------------------------------------------------------------------------//
//! \file qirlightning/LightningQuantum.cc
//---------------------------------------------------------------------------//

#include "LightningQuantum.hh"

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <thread>
#include <utility>
#include <dlfcn.h>

#include "qiree/Assert.hh"

extern "C" Catalyst::Runtime::QuantumDevice*
GenericDeviceFactory(char const* kwargs);
namespace qiree
{
using namespace Catalyst::Runtime;

//---------------------------------------------------------------------------//
/*!
 * Initialize the Lightning simulator
 */
LightningQuantum::LightningQuantum(std::ostream& os, unsigned long int seed)
    : output_(os), seed_(seed)
{
    auto rtld_flags = RTLD_LAZY | RTLD_NODELETE;
    rtd_dylib_handler_ = dlopen(QIREE_LIGHTNING_RTDLIB, rtld_flags);

    QIREE_VALIDATE(rtd_dylib_handler_,
                   << "failed to load Lightning runtime library '"
                   << QIREE_LIGHTNING_RTDLIB << "'");

    // Find device factory
    std::vector<std::string> const factory_names
        = {"LightningSimulatorFactory",
           "LightningKokkosSimulatorFactory",
           "LightningGPUSimulatorFactory"};

    for (auto const& factory_name : factory_names)
    {
        dlerror();
        factory_f_ptr_ = dlsym(rtd_dylib_handler_, factory_name.c_str());
        if (factory_f_ptr_)
        {
            break;
        }
    }

    QIREE_VALIDATE(factory_f_ptr_,
                   << "failed to find valid device factory function");
}

//---------------------------------------------------------------------------//
//! Default destructor
LightningQuantum::~LightningQuantum()
{
    if (rtd_dylib_handler_)
    {
        dlclose(rtd_dylib_handler_);
    }
};

//---------------------------------------------------------------------------//
/*!
 * Prepare to build a quantum circuit for an entry point
 */
void LightningQuantum::set_up(EntryPointAttrs const& attrs)
{
    QIREE_VALIDATE(attrs.required_num_qubits > 0,
                   << "input is not a quantum program");
    num_qubits_ = attrs.required_num_qubits;  // Set the number of qubits
    results_.resize(attrs.required_num_results);

    std::string rtd_kwargs = {};
    rtd_qdevice_ = std::unique_ptr<QuantumDevice>(
        reinterpret_cast<decltype(GenericDeviceFactory)*>(factory_f_ptr_)(
            rtd_kwargs.c_str()));

    rtd_qdevice_->AllocateQubits(num_qubits_);
}

//---------------------------------------------------------------------------//
/*!
 * Complete an execution
 */
void LightningQuantum::tear_down() {}

//---------------------------------------------------------------------------//
/*!
 * Reset the qubit
 */
void LightningQuantum::reset(Qubit q)
{
    q.value = 0;
}

//----------------------------------------------------------------------------//
/*!
 * Read the value of a result.
 */
QState LightningQuantum::read_result(Result r) const
{
    QIREE_EXPECT(r.value < results_.size());
    auto result_bool = static_cast<bool>(results_[r.value]);
    return static_cast<QState>(result_bool);
}

//---------------------------------------------------------------------------//
/*!
 * Map a qubit to a result index.
 */
void LightningQuantum::mz(Qubit q, Result r)
{
    QIREE_EXPECT(q.value < this->num_qubits());
    QIREE_EXPECT(r.value < this->num_results());
    std::mt19937 gen(seed_);
    seed_++;
    rtd_qdevice_->SetDevicePRNG(&gen);
    auto result
        = rtd_qdevice_->Measure(static_cast<intptr_t>(q.value), std::nullopt);
    results_[r.value] = *result;
}

//---------------------------------------------------------------------------//
/*
 * Quantum Instruction Mapping
 */

// 1. Entangling gates
void LightningQuantum::cx(Qubit q1, Qubit q2)
{
    rtd_qdevice_->NamedOperation(
        "CNOT",
        {},
        {static_cast<intptr_t>(q1.value), static_cast<intptr_t>(q2.value)});
}
void LightningQuantum::cnot(Qubit q1, Qubit q2)
{
    rtd_qdevice_->NamedOperation(
        "CNOT",
        {},
        {static_cast<intptr_t>(q1.value), static_cast<intptr_t>(q2.value)});
}
void LightningQuantum::cz(Qubit q1, Qubit q2)
{
    rtd_qdevice_->NamedOperation(
        "CZ",
        {},
        {static_cast<intptr_t>(q1.value), static_cast<intptr_t>(q2.value)});
}
// 2. Local gates
void LightningQuantum::h(Qubit q)
{
    rtd_qdevice_->NamedOperation(
        "Hadamard", {}, {static_cast<intptr_t>(q.value)});
}
void LightningQuantum::s(Qubit q)
{
    rtd_qdevice_->NamedOperation("S", {}, {static_cast<intptr_t>(q.value)});
}
void LightningQuantum::t(Qubit q)
{
    rtd_qdevice_->NamedOperation("T", {}, {static_cast<intptr_t>(q.value)});
}
// 2.1 Pauli gates
void LightningQuantum::x(Qubit q)
{
    rtd_qdevice_->NamedOperation(
        "PauliX", {}, {static_cast<intptr_t>(q.value)});
}
void LightningQuantum::y(Qubit q)
{
    rtd_qdevice_->NamedOperation(
        "PauliY", {}, {static_cast<intptr_t>(q.value)});
}
void LightningQuantum::z(Qubit q)
{
    rtd_qdevice_->NamedOperation(
        "PauliZ", {}, {static_cast<intptr_t>(q.value)});
}
// 2.2 rotation gates
void LightningQuantum::rx(double theta, Qubit q)
{
    rtd_qdevice_->NamedOperation(
        "RX", {theta}, {static_cast<intptr_t>(q.value)});
}
void LightningQuantum::ry(double theta, Qubit q)
{
    rtd_qdevice_->NamedOperation(
        "RY", {theta}, {static_cast<intptr_t>(q.value)});
}
void LightningQuantum::rz(double theta, Qubit q)
{
    rtd_qdevice_->NamedOperation(
        "RZ", {theta}, {static_cast<intptr_t>(q.value)});
}

}  // namespace qiree
