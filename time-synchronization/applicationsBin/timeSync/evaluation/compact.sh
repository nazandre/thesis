#! /bin/bash

echo "Simulator:"
output="compact"
dir="results/fidelity"
./extract-per-distance.sh "$dir/mrtp.dissemination-compact"
Rscript dissemination-error.r

rm -rf "$output"
mv distance "$output"

echo -e "\nHardware:"

(
    cd hardware/quality
    output="distribution-compact"
    rm -rf $output
    ./quality.sh "compact-5003" 2> /dev/null
    mv quality-data $output
)

gnuplot compact.gpl
