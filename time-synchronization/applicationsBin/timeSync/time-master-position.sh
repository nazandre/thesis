#! /bin/bash

n=1
positions=("extremities center centroid betweennessCenter")

workDir="time-master-position"

confDir="$1"
resDir="$workDir/results"

oneMSec=1000
oneSec=$((1000*$oneMSec))
start=$((5*$oneSec))
electionTime=$((3*$oneSec))

winSize=5
period1MRTP=$((2*$oneSec))
period2MRTP=$((5*$oneSec))

SPARSE_COMM_MODEL=0
INTERMEDIATE_COMM_MODEL=1
COMPACT_COMM_MODEL=2
model=$INTERMEDIATE_COMM_MODEL

MRTP=1

declare -A ids

rm -rf "$resDir"
mkdir "$resDir"

for config in $(ls -v $confDir/*.xml); do

    bconfig=$(basename "$config" ".xml")

    subResDir="$resDir/$bconfig/"
    mkdir "$subResDir"
    
    centerFile="$confDir/$bconfig.centers"
    betFile="$confDir/$bconfig.betweenness"

    echo $config

    if [[ ! $config =~ "-x4" ]];
    then
	echo "ignored"
	continue
    fi
    
    for p in $positions; do
	if [ $p == "betweennessCenter" ]; then	
	    ids[$p]=$(grep -i "Betweenness center:" $betFile | cut -d ' ' -f 3 | cut -d ',' -f 1 | tr -d '[:space:]')
	else
	    ids[$p]=$(grep -i "$p:" $centerFile | cut -d ':' -f 2 | cut -d ',' -f 1 | tr -d '[:space:]')
	fi
	echo "$p => id ${ids[$p]}"
    done
    
    for i in $(seq 1 $n); do
	seed="$RANDOM"
	for p in $positions; do 
	    echo -e "\n\tVariant: $i $p"
	    echo -e "\tSeed: $seed"
	    echo -e "\tDate start: $(date)"
	    baseParameters="-c $config -k BB -R -t -B $start -Z $model -N -P $MRTP -L $winSize -F -S $period1MRTP -Q $period2MRTP"
	    parameters="-a $seed $baseParameters -C ${ids[*]} -M ${ids[$p]} $electionTime"
	    echo -e "\t$parameters"
	    ./timeSync $parameters > "$subResDir/$i-$p.output" 2>&1
	    cat "error.dat" >> "$subResDir/error.$p"
	    echo -e "\tDate end: $(date)"
	done
    done
done
