#! /bin/bash

dir="results/fidelity"

predOutputDir="dissemination-error-pred"
rttOutputDir="dissemination-error-rtt"

rm -rf $predOutputDir $rttOutputDir

echo "SIM PRED"
./extract-per-distance.sh "$dir/mrtp.dissemination"
Rscript dissemination-error.r
mv distance/ $predOutputDir

echo -e "\nSIM RTT"
./extract-per-distance.sh "$dir/mrtp.dissemination-rtt"
Rscript dissemination-error.r
mv distance/ $rttOutputDir

(
    cd hardware/quality

    rm -rf "distribution-pred" "distribution-rtt" "distribution-fd"

    echo -e "\nHARDWARE PRED"
    ./quality.sh "data-rate-5003"
    mv "quality-data" "distribution-pred"

    echo -e "\nHARDWARE RTT"
    ./quality.sh "data-rtt"
    mv "quality-data" "distribution-rtt"

    echo -e "\nHARDWARE FD"
    ./quality.sh "fd"
    mv "quality-data" "distribution-fd"
)

#(cd hardware && Rscript dissemination-error.r)

gnuplot dissemination-error.gpl
