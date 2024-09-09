PROJECT_ROOT=$(readlink -f "$(pwd)/..")
MBEDUTILS_ROOT=$(readlink -f "$(pwd)/../mbedutils")
CPPUMOCKGEN=$(readlink -f "$(pwd)/../bin/CppUMockGen")

# Find the latest GCC version. This is known to work with:
# - Ubuntu 22.04 with GCC 11.2.0 and libstdc++-12-dev
LATEST_CPP_VERSION=$(ls -1 /usr/include/c++/ | sort -V | tail -n 1)
LATEST_GCC_VERSION=$(ls -1 /usr/lib/gcc/x86_64-linux-gnu/ | sort -V | tail -n 1)

# Set the INCLUDE_DIRS variable for finding headers
INCLUDE_DIRS="-I /usr/lib/gcc/x86_64-linux-gnu/${LATEST_GCC_VERSION}/include "
INCLUDE_DIRS+="-I /usr/include/c++/${LATEST_CPP_VERSION} "
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

for header in $(find $INTERFACE_DIR -name "*.hpp"); do
    $CPPUMOCKGEN $INCLUDE_DIRS --std c++20 --mock-output $MOCK_DIR --expect-output $EXPECT_DIR --input "$header"
done
