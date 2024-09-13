# Updates the project's submodules in the correct fashion. Some of the submodules
# don't like to be updated with the normal `git submodule update --init --recursive`
# command, so this script is used to update them in the correct way.
PRJ_ROOT=$(pwd)

echo "Initializing core submodules"
git submodule update --init

echo "Updating CppUTest"
cd $PRJ_ROOT/lib/cpputest
git submodule update --init --recursive
