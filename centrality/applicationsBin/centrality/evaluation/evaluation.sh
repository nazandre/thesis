#! /bin/bash

extraction="$1"

if [ $extraction -eq 1 ]; then
    ./extract.sh
fi

./summarize.sh

./plot.sh
