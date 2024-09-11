PROJECT_ROOT=$(readlink -f "$(pwd)/..")
MBEDUTILS_ROOT=$(readlink -f "$(pwd)/../mbedutils")

# Get the location of the CppUMockGen executable
CPPUMOCKGEN=$(readlink -f "$(pwd)/../bin/CppUMockGen")
if [ ! -f $CPPUMOCKGEN ]; then
    echo "CppUMockGen executable not found at $CPPUMOCKGEN - please run build_cppumockgen.sh first."
    exit 1
fi

# Get the location of the gcc include directory
GCC_INSTALL_DIR=$(gcc -print-search-dirs | grep install | cut -d' ' -f2)
GCC_INCLUDE_DIR="${GCC_INSTALL_DIR}include"

# Get the location of the C++ include directory
GCC_VERSION=$(gcc -dumpversion)
LATEST_CPP_VERSION=$(ls -1 /usr/include/c++/ | grep "^$GCC_VERSION" | sort -V | tail -n 1)
CPP_INCLUDE_DIR="/usr/include/c++/$LATEST_CPP_VERSION"

# Set the INCLUDE_DIRS variable for finding headers
INCLUDE_DIRS="-I $GCC_INCLUDE_DIR "
INCLUDE_DIRS+="-I $CPP_INCLUDE_DIR "
INCLUDE_DIRS+="-I $MBEDUTILS_ROOT/include "
INCLUDE_DIRS+="-I $PROJECT_ROOT/lib/etl/include "

# Print the INCLUDE_DIRS for verification
echo "Using INCLUDE_DIRS: ${INCLUDE_DIRS}"

INTERFACE_DIR=$MBEDUTILS_ROOT/include/mbedutils/interfaces
echo "Generating mocks for all header files in $INTERFACE_DIR"

# Get a list of all header files in the $MBEDUTILS_ROOT/include directory and its subdirectories
# and generate mocks for each of them.
MOCK_DIR=$PROJECT_ROOT/tests/mock
EXPECT_DIR=$PROJECT_ROOT/tests/expect

mkdir -p $MOCK_DIR
mkdir -p $EXPECT_DIR

# What does mock().clear() and mock().check_expectations() do?
# Are expectations serialized/order dependent?

# These files either have nothing to mock or have been manually adjusted from the default cppumockgen output
exclusion_file_list=(
    "spi_intf.hpp"
    "util_intf.hpp"
)

for header in $(find $INTERFACE_DIR -name "*.hpp"); do
    # Skip the header file if it is in the exclusion list
    if [[ " ${exclusion_file_list[@]} " =~ " ${header##*/} " ]]; then
        echo "Skipping $header"
        continue
    fi

    $CPPUMOCKGEN $INCLUDE_DIRS --std c++20 --mock-output $MOCK_DIR --expect-output $EXPECT_DIR --input "$header"
done
