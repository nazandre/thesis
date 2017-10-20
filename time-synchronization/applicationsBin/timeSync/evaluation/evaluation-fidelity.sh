#! /bin/bash

dir="results/fidelity"

echo "###### Dissemination error"

./dissemination-error.sh

./no-reg.sh

./compact.sh

echo "###### Delay"
rm -rf "delay"
mkdir "delay"
Rscript delay.r "$dir/mrtp.rtt"
(cd hardware && Rscript one-way.r && ./delay.sh)
gnuplot delay.gpl

#rm -rf "tm-error"
#mkdir "tm-error"
#Rscript synchronization-error.r "$dir/mrtp.l2d-10"
#gnuplot synchronization-error.gpl

echo "###### Period"
Rscript period-hardware.r
Rscript period-visiblesim.r
gnuplot period.gpl

echo "###### Window"
Rscript window-hardware.r
Rscript window-visiblesim.r
gnuplot window.gpl
