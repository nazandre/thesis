#! /bin/bash

# rawData -> data "
rawDataDir="rawData"
dataDir="data"
rm -rf "$dataDir"
mkdir "$dataDir"

echo "rawData => data..."

for robotDir in $(ls -d "$rawDataDir"/*/); do
    robot="$(basename $robotDir)"
    mkdir "$dataDir/$robot"
    echo "$robot"
    for topologyDir in $(ls -d "$robotDir"/*/); do
	topology="$(basename $topologyDir)"
	mkdir "$dataDir/$robot/$topology"	
	echo -e "\tTopology: $topology"	
	for file in $(ls "$topologyDir/all."*); do
	    alg="${file##*.}"
	    Rscript summary.r "$file" > /dev/null 2>&1

	    echo -e "\t$alg"
	    mv time.dat "$dataDir/$robot/$topology/$alg.time"
	    mv messages.dat "$dataDir/$robot/$topology/$alg.messages"
	    mv avgMessages.dat "$dataDir/$robot/$topology/$alg.avgMessages"
	    mv queue.dat "$dataDir/$robot/$topology/$alg.queue"
	    mv errorEcc.dat "$dataDir/$robot/$topology/$alg.errorEcc"
	    mv errorFar.dat "$dataDir/$robot/$topology/$alg.errorFar"
	    mv memory.dat "$dataDir/$robot/$topology/$alg.memory"

	    # Disable by default, because of its high computation and memory costs!
	    # Was used to ensure the correctness of the formula to compute values in
	    dir=$(dirname "$file")
	    iStatsFile="$dir/iStats.$alg"
	    if [ -f $iStatsFile ]; then
		Rscript summaryAvgMessages.r "$iStatsFile" > /dev/null 2>&1
		mv avgMessages2.dat "$dataDir/$robot/$topology/$alg.avgMessages2"
	    fi
	done
    done
done
