#! /bin/bash

resDir="$1"
variants=("mrtp-diameter" "mrtp-diameter-rtt" "mrtp-e2ace" "mrtp-e2ace-rtt" "tpsn-e2ace" "tpsn-e2ace-mle" "tpsn-diameter-mle" "ad-rtt" "ad" "ats" "ftsp-diameter" "wmts" "PulseSync")

evalDir=$(pwd)

(
    cd $resDir

    #dirs="$(ls -d */)"
    dirs="$2"
    for shapeDir in $dirs; do
	(
	    cd "$shapeDir"
	    echo "=== $shapeDir ==="
	    subdirs="$(ls -d */ | sort -n)"

	    rm -f error-summary.*
	    
	    for subdir in $subdirs; do
		(
		    cd "$subdir"
		    echo -e "\t$subdir"

		    base="$(basename $subdir)"
		    d="$(cat $base.ga | grep "Diameter:" | cut -d " " -f 2)"

		    rm -f error-distribution.*
		    
		    for variant in ${variants[@]}
		    do
			errorFile="error.$variant"
			if [ -f "$errorFile" ];then
			    echo -e "\t\t$errorFile"
			    Rscript "$evalDir/precisionAndConvergence.r" "$errorFile"
			    mv "distribution.dat" "error-distribution.$variant"
			    summary="$(cat summary.dat)" 
			    rm -f "summary.dat"
			    echo "$d $summary" >> "../error-summary.$variant"
			else
			    echo "Warning: $shapeDir/$subdir/$errorFile does not exist"
			fi
		    done
		)
	    done
	)
    done
)
