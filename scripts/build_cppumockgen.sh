THIS_DIR=$(pwd)

# Re-create the build directory
CPPUMOCKGEN_ROOT=$(readlink -f "$(pwd)/../lib/cppumockgen")
CPPUMOCKGEN_BUILD_DIR=$CPPUMOCKGEN_ROOT/build

if [ -d "$CPPUMOCKGEN_BUILD_DIR" ]; then
    rm -rf "$CPPUMOCKGEN_BUILD_DIR"
fi

mkdir -p $CPPUMOCKGEN_BUILD_DIR

# Set the LibClang base directory and library path
export LIBCLANG_HOME=/usr/lib/llvm-14

# Build the CppUMockGen
cd $CPPUMOCKGEN_BUILD_DIR
cmake ..
make -j8

# Copy the CppUMockGen.hpp
OUTPUT_HDR=$(readlink -f "$THIS_DIR/../tests/mock")/CppUMockGen.hpp
echo "Copying CppUMockGen.hpp to $OUTPUT_HDR"
cp $CPPUMOCKGEN_ROOT/app/include/CppUMockGen.hpp $OUTPUT_HDR
