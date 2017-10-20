#! /bin/bash

seed="-1"

ABC_CENTER_V1_INDEX="3"
BLINKY_BLOCKS_PARAMETERS="BB"
NUMBER_OF_EXECUTIONS_PER_XML=25

baseDir="simulation-validation"
resDir="$baseDir/results"
rm -rf "$resDir"
mkdir "$resDir"

confDir="$baseDir/config/"

executeAndCopyFiles () { 
    parameters="$1"
    output="$2"

    bn="$(basename $output)"
    version="${bn##*.}"
    dir="$(dirname $output)"
    confNum="${bn%.*}"
    echo -e "\t\t$output"
    ./centrality $parameters > "$output"  2>&1
}

for shapeDir in $(ls -d "$confDir/"*); do
    shape="$(basename $shapeDir)"
    echo "Shape: $shape"
    subResDir="$resDir/$shape"
    mkdir "$subResDir"
    for xml in $(ls -v $shapeDir/*.xml); do
	bxml=$(basename $xml .xml)
	indent="\t"
	echo -e "${indent}Configuration: $bxml, seed: RANDOM"
	for i in $(seq 1 $NUMBER_OF_EXECUTIONS_PER_XML); do
	    seed="$RANDOM"
	    genericParameters="-c $xml -k $BLINKY_BLOCKS_PARAMETERS -i -a $seed -R -t -A $ABC_CENTER_V1_INDEX"
	    genericOutputFile="$subResDir/$bxml-$i.abc-center-v1"
	    executeAndCopyFiles "$genericParameters" "$genericOutputFile"
	done
    done
done
