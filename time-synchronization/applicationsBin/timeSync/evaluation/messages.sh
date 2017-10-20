#! /bin/bash

#variants=("mrtp-diameter" "mrtp-e2ace" "mrtp-e2ace-rtt" "tpsn-e2ace" "ad-rtt" "ad" "ats")
#names=("MRTP-MINID-STATIC" "MRTP-PC2LE-STATIC" "MRTP-PC2LE-RTT" "TPSN-PC2LE" "AD-RTT" "AD" "ATS-STATIC")

#variants=("mrtp-e2ace" "mrtp-diameter" "tpsn-e2ace" "mle-tpsn-e2ace" "ad" "ats")
#names=("MRTP-PC2LE-PRED" "MRTP-MIN_ID-PRED" "TPSN-PC2LE-RTT" "MLE_TPSN-PC2LE-RTT""AD-PRED" "ATS-PRED")

variants=("mrtp-e2ace" "mrtp-diameter" "tpsn-diameter-mle" "wmts" "ats" "PulseSync")
names=("MRTP-PC2LE-PRED" 'MRTP-MIN\\_ID-PRED' 'MLE\\_TPSN-MIN\\_ID' "WMTS-PRED" "ATS-PRED" "PulseSync-PRED")

evalDir=$(pwd)
resDir="$1"
messagesDir="$evalDir/messages"
rm -rf "$messagesDir"
mkdir "$messagesDir"

(
    cd "$resDir"
    dirs="$2"
    #$(ls -d */)"
    for shapeDir in $dirs;
    do
	(
	    cd "$shapeDir"
	    echo "=== $shapeDir ==="

	    shapeMessageDir="$messagesDir/$(basename $shapeDir)"
	    mkdir "$shapeMessageDir"
	    
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

		    messageFile="$shapeMessageDir/$base.dat"
		    
		    echo "Type Leader Infrastructure Synchronization" >> "$messageFile"
		    
		    for ((i=0; i < ${#variants[@]}; i++))
		    do
			echo -e "\t${variants[$i]} => ${names[$i]}"
			variant=${variants[$i]}
			name=${names[$i]}
			output="output.$variant"
			if [ -f "$output" ];then

			    leaderMessages=0
			    treeMessages=0
			    convergecastMessages=0

			    synchronizationMessages="$(cat $output | grep "Number of messages processed:" | cut -d " " -f 5)"
			    
			    # if min id
			    if [[ $variant == *"diameter"* ]]; then
				echo -e "\t\tdiameter!"
				leaderElection="output.min-id"
				leaderMessages="$(cat $leaderElection | grep "Number of messages processed:" | cut -d " " -f 5)"
				treeConstruction="output.tree-diameter"
				treeMessages="$(cat $treeConstruction | grep "Number of messages processed:" | cut -d " " -f 5)"
				if [[ $variant == *"mrtp"* ]]; then
				    convergecast="output.convergecast-diameter"
				    convergecastMessages="$(cat $convergecast | grep "Number of messages processed:" | cut -d " " -f 5)"
				    convergecastMessages=$(($convergecastMessages - $treeMessages))
				    
				    if [[ $variant == *"rtt"* ]]; then
					synchronizationMessages=$(($synchronizationMessages+$convergecastMessages))
					convergecastMessages=$((2*$convergecastMessages))
				    fi
				fi
			    elif [[ $variant == *"e2ace"* ]]; then
				#"$base.e2ace-stats"
				echo -e "\t\te2ace!"
				leaderElection="output.e2ace"
				leaderMessages="$(cat $leaderElection | grep "Number of messages processed:" | cut -d " " -f 5)"
				treeConstruction="output.tree-e2ace"
				treeMessages="$(cat $treeConstruction | grep "Number of messages processed:" | cut -d " " -f 5)"
				if [[ $variant == *"mrtp"* ]]; then
				    convergecast="output.convergecast-e2ace"
				    convergecastMessages="$(cat $convergecast | grep "Number of messages processed:" | cut -d " " -f 5)"
				    convergecastMessages=$(($convergecastMessages - $treeMessages))
				    if [[ $variant == *"rtt"* ]]; then
					synchronizationMessages=$(($synchronizationMessages+$convergecastMessages))
					convergecastMessages=$((2*$convergecastMessages))  
				    fi
				fi
			    fi
			    
			    synchronizationMessages="$(($synchronizationMessages - $convergecastMessages - $treeMessages))"
			    
			    leaderMessages=$(echo $leaderMessages/$n | bc -l)
			    treeMessages=$(echo $treeMessages/$n | bc -l)
			    synchronizationMessages=$(echo "($synchronizationMessages+$convergecastMessages)/$n" | bc -l)
			    
			    echo -e "\t\t$name $leaderMessages $treeMessages $synchronizationMessages"
			    echo "$name  $leaderMessages $treeMessages $synchronizationMessages" >> "$messageFile"
			else
			    echo "Warning: $shapeDir/$subdir/$output does not exist!"
			fi
		    done
		)
	    done
	)
    done
)

#for variant in ${variants[@]}
#	do
#			errorFile="error.$variant"
#			if [ -f "$errorFile" ];then

#			    Rscript "$evalDir/precision.r" "$errorFile"
#			    mv "distribution.dat" "error-distribution.$variant"
#			    summary="$(cat summary.dat)" 
#			    rm -f "summary.dat"
#			    echo "$d $summary" >> "../error-summary.$variant"
#			else
#			    echo "Warning: $shapeDir/$subdir/$errorFile does not exist"
#			fi
#		    done

