#! /bin/bash

output="evaluation.out"

date > $output
echo -e "\n######" >> $output
./evaluation.sh Ball 2>&1 >> $output
echo -e "######\n" >> $output
date >> $output
