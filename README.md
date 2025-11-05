# QIR-EE

The QIR Execution Engine library provides interfaces for processing [Quantum Intermediate Representation](https://github.com/qir-alliance/qir-spec/) code using the LLVM execution engine.

## Introduction

The Quantum Intermediate Representation Execution Engine (QIR-EE, pronounced 'cure-ee') is a tool designed to streamline the process of running quantum circuits and algorithms. One of our goals is to make your journey into quantum computing as seamless as possible. This implementation is associated to the paper [A Cross-Platform Execution Engine for the Quantum Intermediate Representation](https://doi.org/10.1007/s11227-025-07969-2) published in the Journal of Supercomputing, Volume 81, 1521 (2025) and is maintained by the QIR-EE Developers.

This work represents the feasibility of a modular workflow at the lower end of the quantum software stack. We welcome feedback and ideas for collaborations.

## Documentation

Most of the QIR-EE documentation is readable through the codebase through a combination of [static RST documentation](doc/index.rst) and Doxygen-markup comments in the source code itself. The full QIR-EE user documentation (including selected code documentation incorporated by Breathe) and the QIR-EE code documentation will be mirrored on our GitHub pages site. You can generate these yourself (if the necessary prerequisites are installed) by setting the option `-DQIREE_BUILD_DOCS=ON` running `make doc` (user) or `make doxygen` (developer).

- [user-docs (coming soon)](https://ornl-qci.github.io/qir-ee/user/index.html)
- [dev-docs (coming soon)](https://ornl-qci.github.io/qir-ee/dev/index.html)

## Getting Started

### Dependencies

There is a key dependency for QIR-EE to work properly. Please make sure to download and install the most current version of:
1. [LLVM](https://releases.llvm.org/) (we have tested versions 14 through 18).

There is an optional dependency for QIR-EE to run on hardware backends.

2. [XACC](https://github.com/ORNL-QCI/xacc): Here, we recommend setting option `-DQIREE_MINIMAL_BUILD=ON` during cmake for a faster build. Follow the [XACC prerequisites](https://xacc.readthedocs.io/en/latest/install.html) page to ensure a smoother installation.

### Requirements for Basic Runs

- Ability to download and build this project with cmake.
- A QIR file with the quantum program that you want to run (see examples folder).

### (Optional) Credentials for Hardware and Specialized Simulator Runs

- Access to a compatible quantum computing simulator or real quantum hardware.
- Configuration files set up in your home folder that contain credentials to
  access vendor backends. This typically takes on the form `.backend_config` depending on the backends you have access to.

### Installation


QIR-EE Setup in Command Line

1. Clone/download/unzip this repo.

2. Enter the repo folder.

3. `mkdir build; cd build`

4. `cmake ..`

5. `make`

By default, we have the following options. These can be adjusted in step 3.

`-DQIREE_BUILD_TESTS=ON`, `-DQIREE_BUILD_DOCS=OFF`, `-DDQIREE_USE_XACC=ON`, `-DQIREE_USE_QSIM=ON`

The resulting path to executable files can be exported as

`export PATH=${YOUR-QIREE-INSTALL-DIR}/bin:$PATH`

which would allow you to access the QIR-EE from anywhere in command line.

## Executing Quantum Circuits

### (via QIR-EE and QSIM)

Syntax For basic usage:

```
qir-qsim llvm-file-path -s num-shots
```

The default number of shots is 1 if you don't invoke that option.

Example:

```
qir-qsim $HOME/qiree/examples/teleport.ll -s 100
```

This command will execute the teleport algorithm described in `teleport.ll` with 100 shots using Google's qsim state simulator, which can handle mid-circuit measurements.

### (via QIR-EE and XACC)

1. Check that your cmake prefixes for XACC are correct.
2. Typing `echo $CMAKE_PREFIX_PATH` should give you the path to your XACC
   installation.
3. If empty, then add it: `export CMAKE_PREFIX_PATH=$HOME/.xacc` or an
   equivalent path to your XACC installation.
4. Check your `$PYTHONPATH` for pointing to your XACC installation.
5. If empty, then add it: `export PYTHONPATH=$PYTHONPATH:$HOME/.xacc` or an
   equivalent path to your XACC installation.

Syntax For basic usage:

```
qir-xacc llvm-file-path --flag-name flag-value
```
1. `qir-xacc` may be replaced with an equivalent executable.
2. `llvm-file-path` is used to indicate path of the LLVM (`*.ll`) file that
   specifies the quantum program (required).
3. `-a` or `--accelerator` is the name of the quantum accelerator (hardware or
   simulator) that you wish to use (required).
4. `-s` or `--shots` is the number of shots (optional with default at 1024).

   With XACC we have tested: `aer`, `qpp`, `qsim`, `quantinuum:H1-1SC`, `quantinuum:H1-1E`,
   `quantinuum:H1-1`, `ionq`, `ionq:sim.harmony`, `ionq:sim.aria-1`, `ionq:qpu.harmony`.

   Note that the minimal build only includes `aer` and `qpp` for local simulators.

Example:

```
qir-xacc $HOME/qiree/examples/bell.ll --accelerator qpp
```

This command will execute the quantum Bell circuit described in `bell.ll` the (default) 1024 times using the "qpp" accelerator.

## Understanding the Results

After execution, you will receive a summary of the quantum circuit's results. This may include a simple dictionary of counts for qubit strings or the xacc buffer output. Some example outputs from experiments can be found here: [qiree-data](https://github.com/wongey/qiree-data).

## Directory structure

| **Directory** | **Description**                                       |
| ------------- | ---------------                                       |
| **app**       | Source code for installed executable applications     |
| **cmake**     | Implementation code for CMake build configuration     |
| **doc**       | Code documentation and manual                         |
| **examples**  | A collection of QIR examples                          |
| **scripts**   | Development and continuous integration helper scripts |
| **src**       | Library source code                                   |
| **test**      | Unit tests                                            |

## Citation

This code was was made public on github on June 24, 2024 and released to [DOE CODE](https://www.osti.gov/doecode/biblio/149694) on January 15, 2025.
```
@misc{qireerepo,
title = {{QIR-EE}},
author = {{QIR-EE Developers}},
url = {https://github.com/ORNL-QCI/qiree},
howpublished = {[Computer Software] \url{https://doi.org/10.11578/qiree/dc.20250114.1}},
year = {2025},
}
```
