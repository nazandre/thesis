#! /bin/bash

rm -f "simulation-validation/simulation-validation.out"

for i in {1..20}; do
    echo -n "$i..."
    ./simulation-validation.sh > /dev/null 2>&1 && (cd simulation-validation/ && ./simulation-validation.sh 1 >> simulation-validation.out 2>&1)
    echo "done!"
done
