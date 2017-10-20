#! /bin/sh

here=$(pwd)
find "$here" -type f -name "*.cpp" -exec git add {} \;
find "$here" -type f -name "*.h" -exec git add {} \;
