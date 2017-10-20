#! /bin/bash

resDir="$1/"

for conf in $(ls -d $resDir/*); do
    echo $conf
    exec 3>&1
    exec 1> >(paste /dev/null -)
    Rscript summary.r $conf
    exec 1>&3 3>&-
done
