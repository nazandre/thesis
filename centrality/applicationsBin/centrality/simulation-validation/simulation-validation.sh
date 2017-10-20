#! /bin/bash

NUMBER_OF_EXECUTIONS_PER_XML="25"

resDir="rawData"
rm -rf "$resDir"
mkdir "$resDir"

confDir="config"

center="$1"
	   
for shapeDir in $(ls -d "$confDir/"*); do
    shape="$(basename $shapeDir)"
    echo "Shape: $shape"
    subResDir="$resDir/$shape"
    mkdir "$subResDir"
    for xml in $(ls -v $shapeDir/*.xml); do
	bxml=$(basename $xml .xml)
	indent="\t"
	for i in $(seq 1 $NUMBER_OF_EXECUTIONS_PER_XML); do
	    outputFile="results/$shape/$bxml-$i.abc-center-v1"
	    output="$(cat $outputFile)"

	    time=0
	    
	    if [ $center -eq 1 ]; then
		time=$(echo "$output" | grep "Simulator elapsed time" | cut -d ':' -f 2 | cut -d ' ' -f 2)
		time2=$(echo "$output" | grep "has won the election" | cut -d ' ' -f 7 | cut -d '!' -f 1)
		#echo "$outputFile"

		if [ $time -ne $time2 ]; then
		    echo "ERROR: election time ($time) != simulation end time ($time2). File: $outputFile"
		    exit
		fi
	    else
		time=$(echo "$output" | grep "Time to elect A:" | cut -d ':' -f 2 | cut -d 'm' -f 1)
		time=$(echo $time*1000 | bc -l)
	    fi
	    echo $time >> "$subResDir/$bxml.dat"
	   
	done

	stats="$(Rscript simulation-validation.r $subResDir/$bxml.dat)"

	string="$shape/$bxml.txt"
	hardwareSkew=""


	hardwareFile="hardware/stats-skew2.dat"
	if [ $center -eq 1 ]; then
	    hardwareSkew=$(grep -A2 $string $hardwareFile | grep "Center:"| cut -d ":" -f 2 | xargs)
	else
	    hardwareSkew=$(grep -A2 $string $hardwareFile | grep "MinID:"| cut -d ":" -f 2 | xargs)
	fi
	
	echo -e "\t$xml:"
	echo -e "\t\tSim: $stats"
	echo -e "\t\tHard-Skew: $hardwareSkew"
	#echo -e "\t\tHard-NoSkew: $hardwareNoSkew"
    done
done


	
