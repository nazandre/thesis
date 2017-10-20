#! /bin/bash

input="results/fidelity/mrtp.rtt"
outputDir="processing"

rm -rf $outputDir
mkdir -p $outputDir

Rscript processing.r $input

gnuplot processing.gpl
