#!/bin/bash

dir="distance"
input="$1"

rm -rf "$dir"
mkdir "$dir"

IFS=" "
while read -r id sim realGlobal estimatedGlobal local global distance
do
	line="$id $sim $realGlobal $estimatedGlobal $local $global $distance"
	echo $line >> "$dir/$distance.dat"
done < $1
