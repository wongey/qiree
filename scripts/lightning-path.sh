#!/bin/bash -e
#
# This script determines the absolute path to a PennyLane-Lightning simulator
# library to be used with QIREE's Lightning backend.
#
# Usage:
#   ./scripts/lightning-path.sh <simulator_type>
#
# Example:
#   ./scripts/lightning-path.sh qubit

if [ -z "$1" ]; then
  echo "Error: Missing argument. Usage: $0 <simulator_type>" >&2
  echo "Example: $0 qubit" >&2
  exit 1
fi

# Validate simulator type
case "$1" in
  qubit|gpu|kokkos)
    ;;
  *)
    echo "Error: Invalid simulator type '$1'. Must be one of 'qubit', 'gpu', or 'kokkos'." >&2
    exit 1
    ;;
esac

SIM_TYPE="$1"

# Determine OS-specific library suffix
UNAME_S=$(uname -s)
case "$UNAME_S" in
  Linux*)   LIB_SUFFIX=".so";;
  Darwin*)  LIB_SUFFIX=".dylib";;
  *)
    echo "Error: Unsupported platform '$UNAME_S'. QIREE with PennyLane-Lightning only supports Linux and macOS." >&2
    exit 1
    ;;
esac

# Find the base PennyLane-Lightning installation directory using Python
BASE_PATH=$(python -c "import site; print(f'{site.getsitepackages()[0]}/pennylane_lightning')")

if [ -z "$BASE_PATH" ]; then
  echo "Error: Could not determine pennylane_lightning path via Python." >&2
  echo "Is pennylane-lightning installed in your current Python environment?" >&2
  exit 1
fi

# Construct the full library path
LIB_NAME="liblightning_${SIM_TYPE}_catalyst${LIB_SUFFIX}"
FULL_PATH="${BASE_PATH}/${LIB_NAME}"

# Check if the file actually exists
if [ ! -f "$FULL_PATH" ]; then
  echo "Error: Simulator library not found at: $FULL_PATH" >&2
  echo "Ensure you have installed the correct simulator type ('$SIM_TYPE')" >&2
  exit 1
fi

echo "$FULL_PATH"
