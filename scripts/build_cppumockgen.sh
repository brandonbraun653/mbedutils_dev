#Currently this script isn't working. Not sure why it's not finding clang.

# Create the output directory
OUTPUT_DIRECTORY=$(readlink -f "$(pwd)/../bin")
mkdir -p $OUTPUT_DIRECTORY

# Create the build directory
CPPUMOCKGEN_ROOT=$(readlink -f "$(pwd)/../lib/cppumockgen")
CPPUMOCKGEN_BUILD_DIR=$CPPUMOCKGEN_ROOT/build
mkdir -p $CPPUMOCKGEN_BUILD_DIR

# Set the LibClang base directory and library path
export LIBCLANG_HOME=/usr/lib/llvm-14

# Build the CppUMockGen
cd $CPPUMOCKGEN_BUILD_DIR
cmake ..
make -j8

# Copy the CppUMockGen binary
OUTPUT_BIN=$OUTPUT_DIRECTORY/CppUMockGen
echo "Copying to $OUTPUT_BIN"
cp $CPPUMOCKGEN_BUILD_DIR/app/CppUMockGen-0.6.0 $OUTPUT_BIN

# Copy the CppUMockGen.hpp
OUTPUT_HDR=$OUTPUT_DIRECTORY/CppUMockGen.hpp
echo "Copying CppUMockGen.hpp to $OUTPUT_HDR"
cp $CPPUMOCKGEN_ROOT/app/include/CppUMockGen.hpp $OUTPUT_HDR
