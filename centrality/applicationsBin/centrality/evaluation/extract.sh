#! /bin/bash

# Extract data
confDir="confs/confs"
resDir="results"

rawDataDir="rawData/"
rm -rf "$rawDataDir"
mkdir "$rawDataDir"

echo "extracting raw data..."

#for robotDir in $(ls -d "$confDir"/*/); do
robots=("$confDir/bb")
for robotDir in $robots; do
    robot="$(basename $robotDir)"

    if [ ! $robot == "bb" ]; then
	continue
    fi
    
    echo "Robot: $robot"
    vsRobotName="${vsRobotNames[$robot]}"
    mkdir "$rawDataDir/$robot/"
    for topologyDir in $(ls -d "$robotDir"/*/); do
	topology="$(basename $topologyDir)"
	echo -e "\tTopology: $topology"

	#rm -rf "summary.*"
	mkdir "$rawDataDir/$robot/$topology/"
	
	for sizeDir in $(ls -d -v "$topologyDir"/*/); do
	    size="$(basename $sizeDir)"
	    echo -e "\t\tSize: $size"
	   
	    for xml in $(ls -v $sizeDir/*.xml); do
		#seed="$RANDOM"
		bxml=$(basename $xml .xml)
		#echo -e "\t\t\tConfiguration: $bxml"

		conf="confs/confs/$robot/$topology/$size/$bxml"
		theoricResults="$(cat $conf.centers)"

		#echo "$theoricResults"

		theoricEcc=$(echo "$theoricResults" | grep "Radius" | cut -d ':' -f 2 | cut -d ' ' -f 2 )
		theoricFar=$(echo "$theoricResults" | grep "Min Farness" | cut -d ':' -f 2 | cut -d ' ' -f 2 )
		diameter=$(echo "$theoricResults" | grep "Diameter" | cut -d ':' -f 2 | cut -d ' ' -f 2 )
		links=$(echo "$theoricResults" | grep "edges" | cut -d ',' -f 2 | cut -d ' ' -f 2)

		#echo "Extracted: $theoricEcc $theoricFar $diameter $links"
		#exit

		configPath="$resDir/$robot/$topology/$size/$bxml"
		for outputFile in $(ls -v $configPath.*); do
		    #algIndex=${algIndices[$alg]}
		    #outputFile="$resDir/$robot/$topology/$size/$bxml.$alg"
		    		    
		    #if [ ! -f $outputFile ]; then
		    #continue
		    #fi

		    #echo $outputFile
		    
		    alg="${outputFile##*.}"
		    
		    if grep -q "failed" "$outputFile"; then
			echo "ERROR: $outputFile execution has failed! Skip this file."
			continue
		    fi

		    numWinners=$(grep -c "has won the election" "$outputFile") 
		    
		    if [ $numWinners -eq 0 ]; then
			echo "ERROR: $numWinners node elected in $outputFile! Skip this file."
			continue
		    fi

		    if [ $numWinners -ne 1 ] && [[ ! $alg =~ "exact" ]]; then
			echo "ERROR: $numWinners node elected in $outputFile! Skip this file."
			continue
		    fi
		    

		    output="$(cat $outputFile)"
		    #echo "$output"
		    time=0		    
		    error=0
		    
		    if [[ $alg =~ "exact" ]]; then
			time=$(echo "$output" | grep "elected at time" | cut -d ' ' -f 5)
		    else
			time=$(echo "$output" | grep "Simulator elapsed time" | cut -d ':' -f 2 | cut -d ' ' -f 2)
			time2=$(echo "$output" | grep -m 1 "has won the election" | cut -d ' ' -f 7 | cut -d '!' -f 1)
			if [ $time -ne $time2 ]; then
			    echo "WARNING: election time ($time) != simulation end time ($time2)."
			    error=0
			fi
		    fi
		    
		    if [[ $alg =~ "extremum" ]] || [[ $alg =~ "rand-" ]] || [[ $alg =~ "abc-center-v2" ]]; then
			# @12: BFS Completed at 290857 size: 25 ecc: 6
			#echo "Checking BFS..."
			while read -r line ; do
			    #echo "Processing $line"
			    bfsId=$(echo $line | cut -d '@' -f 2 | cut -d ':' -f 1)
			    bfsSize=$(echo $line | cut -d ' ' -f 8)
			    bfsEcc=$(echo $line | cut -d ' ' -f 10)

			    if [ $size -ne $bfsSize ]; then
				error=1
				echo "WRONG SIZE in BFS $bfsId ($bfsSize != $size)"
				break
			    fi

			    bfsRealEcc=$(bin/graphAnalyzer -i $conf.aj -n $bfsId 2>&1 | grep "Node" | cut -d '=' -f 2 | cut -d ' ' -f 2 | cut -d ',' -f 1)

			    if [ $bfsEcc -ne $bfsRealEcc ]; then
				error=1
				echo "WRONG ECC in BFS $bfsId ($bfsEcc != $bfsRealEcc)"
				break
			    fi
			    
			    #echo "id: $bfsId, size: $bfsSize, ecc: $bfsEcc"
			done < <(grep "BFS completed.*ecc" $outputFile)
		    fi

		    initiator=$(grep "Initiator elected.*ecc:" $outputFile)
		    if [ $? -eq 0 ]; then
			#echo "checking initiator!"
			#echo $initiator
			initiatorId=$(echo $initiator | cut -d '@' -f 2 | cut -d ':' -f 1)
			initiatorSize=$(echo $initiator | cut -d ' ' -f 8)
			initiatorEcc=$(echo $initiator | cut -d ' ' -f 10)
			#echo "id: $initiatorId, size: $initiatorSize, ecc: $initiatorEcc"

			if [ $size -ne $initiatorSize ]; then
			    error=1
			    echo "WRONG SIZE in Initiator $initiatorId ($initiatorSize != $size)"
			fi
			
			initiatorRealEcc=$(bin/graphAnalyzer -i $conf.aj -n $initiatorId 2>&1 | grep "Node" | cut -d '=' -f 2 | cut -d ' ' -f 2 | cut -d ',' -f 1)
			if [ $initiatorEcc -ne $initiatorRealEcc ]; then
			    error=1
			    echo "WRONG ECC in Initiator $initiatorId ($initiatorEcc != $initiatorRealEcc)"
			fi
		    fi

		    # elected ecc/farness
		    electedNodeId=$(echo "$output" | grep "has won the election" | cut -d '@' -f 2 | cut -d ':' -f 1)
		    #echo "Elected: $electedNodeId"
		    ga="$(bin/graphAnalyzer -i $conf.aj -n $electedNodeId 2>&1)"
		    #echo "$ga"
		    electedEcc=$(echo "$ga" | grep "Node" | cut -d '=' -f 2 | cut -d ' ' -f 2 | cut -d ',' -f 1)
		    electedFarness=$(echo "$ga" | grep "Node" | cut -d '=' -f 3 | cut -d ' ' -f 2)

		    if [ $alg == "exact-centroid" ]; then
			if [ $electedFarness -ne $theoricFar ]; then
			    echo "ERROR: Exact has elected a non-centroid node of farness $electedFarness/$theoricFar"
			    error=1
			fi
		    fi

		    if [ $error -eq 1 ]; then
		        echo "ERROR: $outputFile execution has failed! Skip this file."
			continue
		    fi
		    
		    ## messages
		    messages=$(echo "$output" | grep "Number of messages processed" | cut -d ':' -f 2 | cut -d ' ' -f 2)

		    ## avg messages
		    
		    #Format: "parameter: min mean max standard-deviation"
		    #Received messages: 26 69.320000 131 28.038145
		    avgMessages=$(echo "$output" | grep "Sent messages" | cut -d ':' -f 2 | cut -d ' ' -f 3)
		    sdMessages=$(echo "$output" | grep "Sent messages" | cut -d ':' -f 2 | cut -d ' ' -f 5)
		  
		    ## queue
		    queue=$(echo "$output" | grep "Maximum message queue size" | cut -d ' ' -f 7)
		    #maxQueueOut=$(echo "$output" | grep "Maximum outgoing message queue size" | cut -d ' ' -f 8)
		    #maxQueueIn=$(echo "$output" | grep "Maximum incomming message queue size" | cut -d ' ' -f 8)
		    # For now (before fixing bugs)
		    #queue=$(echo "$output" | grep "Maximum outgoing message queue size" | cut -d ':' -f 8)
		    #queue=$(($queue + 6))

		    ## memory
		    memory=$(echo "$output" | grep "Algorithm-specific variable memory usage per node:" | cut -d ' ' -f 7)
		    	    
		    #echo "Extracted alg: $time $messages $queue $electedEcc $electedFarness"
		    #exit
		    echo "$size $links $diameter $theoricEcc $theoricFar $time $messages $queue $electedEcc $electedFarness $avgMessages $sdMessages $memory" >> "$rawDataDir/$robot/$topology/all.$alg"

		    iStatsFile="$resDir/$robot/$topology/$size/$bxml-iStats.$alg"
		    if [ -f $iStatsFile ]; then
			sed -e "s/^/$size /" "$iStatsFile" >> "$rawDataDir/$robot/$topology/iStats.$alg"
		    fi		    
		done
	    done
	done	
    done
done
