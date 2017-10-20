#! /bin/bash

variants=("mrtp-e2ace" "mrtp-diameter" "tpsn-e2ace" "tpsn-e2ace-mle" "wmts" "ats")
names=("MRTP-PC2LE-STATIC" "MRTP-MINID-STATIC" "TPSN-PC2LE-RTT" "MLE_TPSN-PC2LE-RTT" "WMTS" "ATS-STATIC")

evalDir=$(pwd)
resDir="$1"
queueDir="$evalDir/queue"
rm -rf "$queueDir"
mkdir "$queueDir"

function maxQueueVariant {
    file="$1"
    echo "$(Rscript $evalDir/queue.r $file)"
}


(
    cd "$resDir"
    dirs="$2"
    #dirs="Ball"
    #$(ls -d */)"
    for shapeDir in $dirs;
    do
	(
	    cd "$shapeDir"
	    echo "=== $shapeDir ==="
	    
	    subdirs="$(ls -d */ | sort -n)"
	    
	    for subdir in $subdirs;
	    do
		(
		    cd "$subdir"
		    echo -e "$subdir"

		    base="$(basename $subdir)"
		    d="$(cat $base.ga | grep Diameter: | cut -d " " -f 2)"
		    n="$(cat $base.ga | grep Graph: | cut -d " " -f 2)"
		    m="$(cat $base.ga | grep Graph: | cut -d " " -f 4)"

		    echo "Graphs: $n nodes, $m edges, $d diameter"
		    
		    for ((i=0; i < ${#variants[@]}; i++))
		    do
			echo -e "\t${variants[$i]} => ${names[$i]}"
			variant=${variants[$i]}
			name=${names[$i]}
			output="$queueDir/queue.$variant"
			stats="stats.$variant"
			
			if [ -f "$stats" ];then

			    leaderQueue=0
			    syncQueue=18
			    #$(maxQueueVariant $stats)
			    maxQueue=0

			    # Leader election + tree
			    if [[ $variant == *"diameter"* ]]; then
				echo -e "\t\tdiameter!"
				leaderElection="stats.min-id"
				leaderQueue=18
				#$(maxQueueVariant $leaderElection)
			    elif [[ $variant == *"e2ace"* ]]; then
				#"$base.e2ace-stats"
				echo -e "\t\te2ace!"
				leaderElection="stats.e2ace"
				leaderQueue=18
				#$(maxQueueVariant $leaderElection)
			    fi

			    echo -e "\t\t$name $leaderQueue $syncQueue"
			    maxQueue="$(($leaderQueue>$syncQueue?$leaderQueue:$syncQueue))"    
			    echo -e "\t\tmax: $n $maxQueue => $output"

			    echo "$n $maxQueue" >> "$output"

			else
			    echo "Warning: $shapeDir/$subdir/$stats does not exist!"
			fi
		    done
		)
	    done
	)
    done
)

#variants=("mrtp-diameter" "mrtp-e2ace" "tpsn-e2ace" "mle-tpsn-e2ace" "ad" "ats")
#names=("MRTP-MINID-STATIC" "MRTP-PC2LE-STATIC" "TPSN-PC2LE" "MLE_TPSN-PC2LE-RTT" "AD" "ATS-STATIC")

variants2=("mrtp-diameter" "mrtp-e2ace" "tpsn-e2ace-mle" "wmts" "ats")
names2=("MRTP-MIN\_ID-PRED" "MRTP-PC2LE-PRED" "MLE_TPSN-PC2LE-RTT" "WMTS-PRED" "ATS-PRED")

# hack for histograms
balls=(10 30 54)
for ((i=0; i < ${#balls[@]}; i++))
do
    ball="${balls[$i]}"
    #echo "Ball $ball"
    outputFile="$queueDir/queue-Ball-$ball.dat"
    echo "${names[@]}" > $outputFile
    for ((j=0; j < ${#variants2[@]}; j++))
     do
	 variant="${variants2[$j]}"
	 file="queue/queue.$variant"
	 line="$(($i+1))p"
	 column=2

	 extractedLine=$(sed -n $line < $file)
	 queue=$(echo $extractedLine | cut -d ' ' -f $column) 
	 echo -n "$queue " >> $outputFile 
     done
    echo "" >> $outputFile
done
