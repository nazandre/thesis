#! /bin/sh

here=$(pwd)
find "$here" -type f -name "*~" -exec rm -f {} \;
find "$here" -type f -name "*.depends" -exec rm -f {} \;
find "$here" -type f -name "*.o" -exec rm -f {} \;
