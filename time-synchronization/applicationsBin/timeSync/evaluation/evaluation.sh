#! /bin/bash

dir="results"

#Ball
#l2d-2
shape="$1"

./precisionAndConvergence.sh $dir $shape
(
    cd "$dir/$shape"
    gnuplot ../../precision.gpl
    gnuplot ../../precision2.gpl
    gnuplot ../../precision3.gpl
    mv precision.pdf ../../graphs/
    mv precision2.pdf ../../graphs/
    mv precision3.pdf ../../graphs/
)

gnuplot convergence.gpl

./messages.sh results $shape
gnuplot messages.gpl
gnuplot messages3.gpl

./error-time.sh results
gnuplot error-time-all.gpl
gnuplot error-time-all-2x3.gpl

./queue.sh results $shape
gnuplot queue.gpl
gnuplot queueHistogram.gpl
