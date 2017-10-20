#! /bin/bash

seed="-1"
sizeSkipExact="1000"
#sizeSkipExact="10000"
sizeSkipBegin="0"
sizeSkipFastTest="50000"

numMaxConfigs="50"
#numMaxConfigs="5000"

declare -A algIndices=( ["min-id"]="1" ["sp-min-id"]="2" ["abc-center-v1"]="3" ["abc-center-v2"]="4" ["e2ace"]="5" ["tbce"]="6" ["exact"]="7" ["korach"]="8" ["rand"]="9" ["extremum"]="10" ["dee"]="12" ["rand2"]="13" ["parallelRand"]="14" ["sp-bfs"]="15" ["aplha-sync-bfs-sp"]="16")

declare -A versionIndices=( ["center"]="1" ["centroid"]="2" )
declare -A pCounterIndices=( ["fm"]="1" ["fm-m"]="2" ["hll"]="3" ["hllh"]="4")
declare -A hashFuncIndices=(["affine"]="1" ["affine-prime"]="2" ["knuth"]="3" ["murmur3"]="4" ["fnv1"]="5" ["rand"]="6")

#{"BB", "RB", "SB", "C2D", "C3D", "MR"}
declare -A vsRobotNames=( ["bb"]="BB" ["sb"]="SB" ["c2"]="C2D" ["c3"]="C3D" )

# Algorithms and version to test here (modify this list to restrict the execution to some algs only)

#algorithms=("min-id" "abc-center-v1" "abc-center-v2" "e2ace" "tbce" "exact" "extremum" "rand" "dee" "rand2" "parallelRand")
#algorithms=("sp-min-id" "sp-bfs")
#algorithms=("e2ace" "dee")
#algorithms=("sp-min-id" "sp-bfs" "aplha-sync-bfs-sp")
#algorithms=("abc-center-v2" "extremum" "rand")

algorithms=("e2ace")
versions=("center" "centroid")

# Probabilistic counters and hash funcs

#pCounters=("fm fm-m hll hllh")
#hllHashFuncs=("affine affine-prime knuth murmur3 fnv1 rand")
#fmHashFuncs=("affine affine-prime murmur3 rand")
#numHashFuncs=("1" "0")

#pCounters=("fm fm-m hll")
#hllHashFuncs=("knuth")
fmHashFuncs=("affine")
numHashFuncs=("0")

hllHashFuncs=("knuth")
pCounters=("hll")

resDir="evaluation/results"
rm -rf "$resDir"
mkdir "$resDir"

confDir="evaluation/confs/confs"

executeAndCopyFiles () { 
    parameters="$1"
    output="$2"

    bn="$(basename $output)"
    version="${bn##*.}"
    dir="$(dirname $output)"
    confNum="${bn%.*}"
    
    indent="\t\t\t\t\t"
    echo -e "$indent$output"
    indent="\t$indent"
    echo -e "${indent}Date start: $(date)"

    ./centrality $parameters > "$output"  2>&1

    #mv "individualStats.dat" "$dir/$confNum-iStats.$version"
    #echo -e "${indent}mv individualStats.dat to $dir/$confNum-iStats.$version"

    echo -e "${indent}Date end: $(date)"
}

for robotDir in $(ls -d "$confDir"/*/); do
#for robotDir in $(ls -d "$confDir/b"*); do
    robot="$(basename $robotDir)"
    echo "Robot: $robot"
    subResDir="$resDir/$robot"
    mkdir "$subResDir"
    vsRobotName="${vsRobotNames[$robot]}"
    for topologyDir in $(ls -d "$robotDir"/*/); do
	topology="$(basename $topologyDir)"
	echo -e "\tTopology: $topology"
	subResDir="$resDir/$robot/$topology"
	mkdir "$subResDir"
	for sizeDir in $(ls -d -v "$topologyDir"/*/); do
	    size="$(basename $sizeDir)"
	    echo -e "\t\tSize: $size"
	    
	    if [ $size -le $sizeSkipBegin ]; then
		echo -e "\t\t\tSkipping to jump to systems with size > $sizeSkipBegin (<=  $sizeSkipFastTest modules)."
		continue
	    fi
	    if [ $size -gt $sizeSkipFastTest ]; then
		echo -e "\t\t\tSkipping for fast testing (>  $sizeSkipFastTest modules)."
		continue
	    fi
	    subResDir="$resDir/$robot/$topology/$size"
	    mkdir "$subResDir"
	    for xml in $(ls -v $sizeDir/*.xml); do
		bxml=$(basename $xml .xml)
		
		if [ $bxml -gt $numMaxConfigs ]; then
		    break
		fi
		
		seed="$RANDOM"
		echo -e "\t\t\tConfiguration: $bxml, seed: $seed"
		for alg in ${algorithms[@]}; do
		    algIndex=${algIndices[$alg]}
		    echo -e "\t\t\t\tAlgorithm: $alg (vs index: $algIndex)"
		    if [ $alg == "exact" ] && [ $size -gt $sizeSkipExact ]; then
			echo -e "\t\t\t\t\tSkipping (>  $sizeSkipExact modules)."
			continue
		    fi

		    genericParameters="-c $xml -k $vsRobotName -i -a $seed -R -t -A $algIndex"
		    genericOutputFile="$subResDir/$bxml.$alg"

		    if [ $alg == "sp-min-id" ] || [ $alg == "sp-bfs" ]; then
			executeAndCopyFiles "$genericParameters" "$genericOutputFile"
			executeAndCopyFiles "$genericParameters -U" "$genericOutputFile-ub"
		    elif [ $alg == "e2ace" ] || [ $alg == "exact" ] || [ $alg == "korach" ] || [ $alg = "rand" ] || [ $alg == "extremum" ] || [ $alg == "dee" ] || [ $alg == "rand2" ] || [ $alg = "parallelRand" ]; then
			for version in ${versions[@]}; do
			    versionIndex=${versionIndices[$version]}
			    versionParameters="$genericParameters -V $versionIndex"
			    versionOutputFile="$genericOutputFile-$version"

			    if [ $alg == "exact" ] && [ $version == "center" ]; then
				continue
			    fi
			    
			    if  [ $alg == "e2ace" ]; then
				for pCounter in ${pCounters[@]}; do
				    pCounterIndex=${pCounterIndices[$pCounter]}
				    pCounterParameters="$versionParameters -P $pCounterIndex"
				    pCounterOutputFile="$versionOutputFile-$pCounter"

				    if [ $pCounter == "fm" ] || [ $pCounter == "fm-m" ]; then
					for numHashFunc in ${numHashFuncs[@]}; do
					    for hashFunc in ${fmHashFuncs[@]}; do
						hashFuncIndex=${hashFuncIndices[$hashFunc]}
						hashFuncParameters="$pCounterParameters -H $hashFuncIndex $numHashFunc"
						hashFuncOutputFile="$pCounterOutputFile-$hashFunc-$numHashFunc"
						b=2
						#for b in $(seq 2 2 4); do
						bitStringSizeParameters="$hashFuncParameters -B $b"
						bitStringSizeOutputFile="$hashFuncOutputFile-$b"
						
						executeAndCopyFiles "$bitStringSizeParameters" "$bitStringSizeOutputFile"
						#executeAndCopyFiles "$bitStringSizeParameters -I" "$bitStringSizeOutputFile-random"
						#done
					    done
					done
				    else
					for hashFunc in ${hllHashFuncs[@]}; do
					    hashFuncIndex=${hashFuncIndices[$hashFunc]}
					    hashFuncParameters="$pCounterParameters -H $hashFuncIndex 0"
					    hashFuncOutputFile="$pCounterOutputFile-$hashFunc-0"

					    executeAndCopyFiles "$hashFuncParameters" "$hashFuncOutputFile"
					    #executeAndCopyFiles "$hashFuncParameters -I" "$hashFuncOutputFile-random"
					done
				    fi
				done
			    else
			        executeAndCopyFiles "$versionParameters" "$versionOutputFile"
			    fi
			done
		    else
			executeAndCopyFiles "$genericParameters" "$genericOutputFile"
		    fi
		done
	    done
	done
    done
done
