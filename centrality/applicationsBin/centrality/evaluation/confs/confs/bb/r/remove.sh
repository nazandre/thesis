#!/bin/bash

for dir in */; do
    (
	cd $dir
	for i in $(seq 51 300); do
	    rm -rf $i.*
	done
    )
done
