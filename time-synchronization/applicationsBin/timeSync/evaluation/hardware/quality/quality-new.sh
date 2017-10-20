#! /bin/bash

dir="$(basename $1)"

./extract-quality-new.sh "$dir"
Rscript quality-estimation.r
gnuplot quality-estimation-color.gpl

mv quality-estimation-color.pdf "$dir.pdf"
