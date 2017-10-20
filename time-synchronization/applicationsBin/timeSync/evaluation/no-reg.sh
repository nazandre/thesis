#! /bin/bash

echo "Simulator:"
output="no-reg"
dir="results/fidelity"
./extract-per-distance.sh "$dir/mrtp.dissemination-no-reg"
Rscript dissemination-error.r

rm -rf "$output"
mv distance "$output"

echo -e "\nHardware:"

(
    cd hardware/quality
    output="distribution-no-reg"
    rm -rf $output
    ./quality.sh "data-no-reg" 2> /dev/null
    mv quality-data $output
)

gnuplot no-reg.gpl
