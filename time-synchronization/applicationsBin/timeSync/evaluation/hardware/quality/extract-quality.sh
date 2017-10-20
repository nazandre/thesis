#! /bin/bash

#remove first data
#remove block 26 data

#echo "processing data..."

dir="$1"

outputDir="quality-data"
rm -rf "$outputDir"
mkdir "$outputDir"

# 26 1231 1226 1226 0 3
while IFS=" " read -r identifier globalT localT eGlobalT lastSync level
do
    line="$identifier $globalT $localT $eGlobalT $lastSync $level"
    #echo $line
    #if [ $globalT -gt $((35*60*1000)) ] && [ $globalT -lt $((60*60*1000)) ]
    #then
    #    exit
    #else
        echo $line >> "$outputDir/$level.dat"
    #fi
    
done < $dir/global.dat
#echo "done!"
