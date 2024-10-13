# Common Directories
PROJECT_ROOT=$(readlink -f "$(pwd)/..")
MBEDUTILS_ROOT=$(readlink -f "$(pwd)/../mbedutils")

# Deduce a relative path to the file to mock, which will end up
# being the same mock path.
INPUT_FILE_TO_MOCK=$1
if [ -z "$INPUT_FILE_TO_MOCK" ]; then
    echo "Usage: $0 <file_to_mock>"
    exit 1
fi

# Make sure the file is a .h or .hpp extension
if [[ ! "$INPUT_FILE_TO_MOCK" =~ \.(h|hpp)$ ]]; then
    echo "Error: Input file must have a .h or .hpp extension."
    exit 1
fi

# Get the location of the CppUMockGen executable
CPPUMOCKGEN=$(readlink -f "$(pwd)/../lib/cppumockgen/build/app/CppUMockGen")
if [ ! -f $CPPUMOCKGEN ]; then
    echo "CppUMockGen executable not found at $CPPUMOCKGEN - please run build_cppumockgen.sh first."
    exit 1
fi

# Get the location of the gcc include directory
GCC_INSTALL_DIR=$(gcc -print-search-dirs | grep install | cut -d' ' -f2)
GCC_INCLUDE_DIR="${GCC_INSTALL_DIR}include"

# Get the location of the C++ include directory matching the gcc version
GCC_VERSION=$(gcc -dumpversion)
LATEST_CPP_VERSION=$(ls -1 /usr/include/c++/ | grep "^$GCC_VERSION" | sort -V | tail -n 1)
#CPP_INCLUDE_DIR="/usr/include/c++/$LATEST_CPP_VERSION"
CPP_INCLUDE_DIR="/usr/include/c++/12"

# Set the INCLUDE_DIRS variable for finding headers
INCLUDE_DIRS="-I $GCC_INCLUDE_DIR "
INCLUDE_DIRS+="-I $CPP_INCLUDE_DIR "
INCLUDE_DIRS+="-I $MBEDUTILS_ROOT/include "
INCLUDE_DIRS+="-I $PROJECT_ROOT/lib/etl/include "

# Print the INCLUDE_DIRS for verification
echo "Using INCLUDE_DIRS: ${INCLUDE_DIRS}"

# Get a list of all header files in the $MBEDUTILS_ROOT/include directory and its subdirectories
# and generate mocks for each of them.
MOCK_DIR=$PROJECT_ROOT/lib/mbedutils_test/mock
EXPECT_DIR=$PROJECT_ROOT/lib/mbedutils_test/expect

mkdir -p $MOCK_DIR
mkdir -p $EXPECT_DIR

# Generate the mock/expect outputs
$CPPUMOCKGEN $INCLUDE_DIRS --std c++20 --mock-output "$MOCK_DIR" --expect-output "$EXPECT_DIR" --input "$INPUT_FILE_TO_MOCK"
