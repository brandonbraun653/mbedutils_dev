#!/bin/bash
# Builds the C and Python bindings for the proto files in this directory. It's not likely this script will need to be run manually,
# as the definitions are already included in the repository. However, if you need to make changes to the proto files, you can run
# this script to regenerate the bindings.
#
# It expects a few dependencies to be installed:
# - protobuf
# - grpcio-tools
# - mypy-protobuf
# - nanopb  (This should be found locally in the lib/nanopb directory)
#
# You can install these system wide with the following command:
#   pip install protobuf grpcio-tools mypy-protobuf
#
# However, It's recommended to use a project-specific virtual environment,
# then calling this script from within it. This way, you can avoid conflicts
# with other projects that may require different versions of these dependencies.

# Expects to be called from this directory
_cwd=$(pwd)

SRC_DIR=$_cwd
C_DST_DIR=$_cwd

# Find the nanopb installation directory in the virtual environment
NPB_ROOT=$(python -c "import os; import nanopb; print(os.path.dirname(nanopb.__file__))")
if [ -z "$NPB_ROOT" ]; then
    echo "Could not find nanopb installation directory. Make sure you have the nanopb package installed."
    exit 1
fi

echo "Found nanopb: $NPB_ROOT"
NPB_INC=$NPB_ROOT/generator/proto

# Build the C bindings
python "$NPB_ROOT"/generator/nanopb_generator.py --cpp-descriptors --output-dir="$C_DST_DIR" --proto-path="$SRC_DIR" test_kv_db.proto
