#!/bin/bash

dir="id"
input="$1"

rm -rf "$dir"
mkdir "$dir"

IFS=" "
while read -r id sim realGlobal estimatedGlobal local global distance
do
	line="$id $sim $realGlobal $estimatedGlobal $local $global $distance"
	echo $line >> "$dir/$id.dat"
done < $1
