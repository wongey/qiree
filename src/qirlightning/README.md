# QIR-EE with Lightning simulator backend

The [PennyLane-Lightning](https://github.com/PennyLaneAI/pennylane-lightning) plugins are high-performance quantum simulators, which are part of the [PennyLane](https://github.com/PennyLaneAI/pennylane) ecosystem. The simulators include the following backends (which can be used with QIREE):
- `lightning.qubit`: a fast state-vector simulator with optional OpenMP additions and parallelized gate-level SIMD kernels.
- `lightning.gpu`: a state-vector simulator based on the NVIDIA cuQuantum SDK.
- `lightning.kokkos`: a state-vector simulator written with Kokkos. It can exploit the inherent parallelism of modern processing units supporting the OpenMP, CUDA or HIP programming models.

## Installing a Lightning simulator

For more information on installing Pennylane Lightning simulators from source, please visit the [Lightning installation page](https://docs.pennylane.ai/projects/lightning/en/latest/dev/installation.html).

**Note:** QIREE is tested to work with PennyLane Lightning simulators v0.43.

### Quick start

The easiest way to get started is to install a Lightning simulator (`pennylane-lightning`/`pennylane-lightning-gpu`/`pennylane-lightning-kokkos`) from PyPI via pip:

```
$ pip install pennylane-lightning-kokkos==0.43.0

$ pip show pennylane-lightning-kokkos
Name: PennyLane_Lightning_Kokkos
Version: 0.43.0
Summary: PennyLane-Lightning plugin
Home-page: https://github.com/PennyLaneAI/pennylane-lightning
Author:
Author-email:
License: Apache License 2.0
Location: <site packages path>
Requires: pennylane, pennylane-lightning
```

**Note:** PennyLane and PennyLane lightning supports Python 3.11-3.13.

Running `pip install pennylane` or `pip install pennylane-lightning` will automatically install the `lightning.qubit` (CPU) simulator, and other simulators can be installed by running `pip install pennylane-lightning-kokkos / pennylane-lightning-gpu`.

**Note:** By default, the pre-built `lightning.kokkos` wheels from pip are built with Kokkos OpenMP enabled for CPU. To build Kokkos for other devices (e.g. CUDA or HIP GPUs), please install from source. Instruction can be found [here](https://docs.pennylane.ai/projects/lightning/en/latest/lightning_kokkos/installation.html).

When installing Pennylane-Lightning from pip or from source, you will have the shared libraries for each of the simulator installed. These are named `liblightning_qubit_catalyst.so`/`liblightning_kokkos_catalyst.so`/`liblightning_GPU_catalyst.so` respectively.

To obtain the path to the library:
```
$ export PL_PATH=$(python -c "import site; print( f'{site.getsitepackages()[0]}/pennylane_lightning')")

$ ls $PL_PATH
... liblightning_qubit_catalyst.so  liblightning_kokkos_catalyst.so ...
```

The helper script `qiree/scripts/lightning-path.sh <device>` can be used to obtain the absolute path of the shared library.

## Compile QIR-EE with Lightning backend

To compile QIR-EE with lightning backend:

```
cd qiree/

# Set the path for the lightning simulator shared library using the
# helper script. Update <device> to qubit / gpu / kokkos as required.

export LIGHTNING_SIM_PATH=$(bash ./scripts/lightning-path.sh <device>)

# Proceed with usual build instructions
# but with the extra `-DQIREE_USE_LIGHTNING=ON` and
# `-DQIREE_LIGHTNING_SIM_PATH` cmake flags

mkdir build; cd build
cmake -DQIREE_USE_LIGHTNING=ON -DQIREE_LIGHTNING_SIM_PATH=$LIGHTNING_SIM_PATH ..
make

```

**Note:**
- when running with `lightning.gpu` simulator for Nvidia GPUs, include `cuquantum` libraries in the library path (which will be installed as a dependency from Python), i.e.

```
export LD_LIBRARY_PATH=$(python -c "import site; print( f'{site.getsitepackages()[0]}/cuquantum')")/lib:$LD_LIBRARY_PATH
```


## Running the example

To run (in the `build` directory):

```
$ ./bin/qir-lightning ../examples/bell.ll -s 100
{"00":43,"11":57}
```
