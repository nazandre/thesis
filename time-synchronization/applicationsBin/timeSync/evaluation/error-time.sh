#! /bin/bash

resDir="$1"
evalDir="$(pwd)"
(
    cd $resDir
    
    dirs="$(ls -d */)"
    for shapeDir in $dirs; do
	(
	    cd "$shapeDir"
	    echo "=== $shapeDir ==="
	    subdirs="$(ls -d */ | sort -n)"

	    for subdir in $subdirs; do
		(
		    cd "$subdir"
		    echo -e "\t$subdir"

		    errorFiles="$(ls -f error.* | sort -n)"
		    for errorFile in $errorFiles; do
			echo -e "\t\t$errorFile"
			#tail -n +3 $errorFile > tmp
			#mv tmp $errorFile
			 #if grep -q "0 0 0 0 0" "$errorFile"; then
			if grep -q "0 0" "$errorFile"; then
			    echo "" > /dev/null
			     # lazy to search logical non operator
			 else
			     sed -i '1s/^/1 1\n/' $errorFile
			     sed -i '1s/^/0 0\n/' $errorFile
			 fi
		    done
		    echo -e "\t\tGnuplot..."
		    gnuplot "$evalDir/error-time.gpl"
		    mv error-time.pdf "$evalDir/graphs/$(basename $shapeDir)-$(basename $subdir)-error-time.pdf"

		)
	    done
	)
    done
)
