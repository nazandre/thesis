#! /bin/bash

p=5000000
./timeSync -c "$1" -k BB -a -1 $2 -B $p -Z 2 -N -P 1 -L 5 -F -S $p -Q $p -M 1 $p
