#! /bin/bash

shapes="$1"

seed=20

robot="BB"

SPARSE_COMM_MODEL=0
INTERMEDIATE_COMM_MODEL=1
COMPACT_COMM_MODEL=2
model=$COMPACT_COMM_MODEL

MRTP=1
TPSN=2
TPSNMLE=3
FTSP=4
AD=5
ModifiedATS=6
WMTS=7
TREE=8
CONVERGECAST=9
PULSESYNC=10
GTSP=11

confDir="confs/$shapes/"
resDir="evaluation/results/$shapes/"

rm -rf "$resDir"
mkdir -p "$resDir"

#rtt vs static
#linear regression 5
#e2ace vs min-id

oneSec=1000000
oneMin=$((60*$oneSec))
oneHour=$((60*$oneMin))

winSize=5
period1MRTP=$((2*$oneSec))
period2MRTP=$((5*$oneSec))

period1TPSN=$((5*$oneSec))
winSizeFTSP=5
period1FTSP=$((5*$oneSec))

winSizePULSESYNC=5
period1PULSESYNC=$((5*$oneSec))

period1AD=$((5*$oneSec))
period1ATS=$((5*$oneSec))
period1WMTS=$((5*$oneSec))

period1GTSP=$((5*$oneSec))


start="$oneHour"

clockModelOption=""

executeAndCopyFiles () { 
    xml="$1"
    variant="$2"
    subResDir="$3"
    parameters="$4"
    
    baseParameters="-c $xml -k $robot -a $seed -i -R -t -B $start $clockModelOption -Z $model -N"
    echo -e "\n\tVariant: $variant"
    echo -e "\tParams: $baseParameters $parameters"
    echo -e "\tDate start: $(date)"
    
    ./timeSync $baseParameters $parameters $clockModelOption > "$subResDir/output.$variant"  2>&1

    mv "error.dat" "$subResDir/error.$variant"
    mv "$stats" "$subResDir/stats.$variant"

    echo -e "\tDate end: $(date)"
}

for xml in $(ls $confDir*.xml); do
#for xml in $(ls $confDir/54.xml); do    
    echo "$xml"
    
    xmlBaseName="$(basename $xml .xml)"

    subResDir="$resDir$xmlBaseName"

    mkdir $subResDir

    cp "$confDir$xmlBaseName.ga" $subResDir/
    cp "$confDir$xmlBaseName.min-id" "$subResDir/output.min-id"
    cp "$confDir$xmlBaseName.e2ace" "$subResDir/output.e2ace"
    cp "$confDir$xmlBaseName.min-id-stats" "$subResDir/stats.min-id"
    cp "$confDir$xmlBaseName.e2ace-stats" "$subResDir/stats.e2ace"

    # enable macro: rtt + export MRTP
    # Comparisons + scalability
    minIdFile="$confDir/$xmlBaseName.min-id"    

    #minId=$(cat "$minIdFile" | grep "has won the election" | cut -d " " -f 1 | cut -d "@" -f 2)
    #new format:
    minId=$(cat "$minIdFile" | grep "has won the election" | cut -d ":" -f 1 | cut -d "@" -f 2)
    
    minIdTime=$(cat "$minIdFile" | grep "has won the election" | cut -d " " -f 7| cut -d "!" -f 1)
    
    diamIdFile="$confDir/$xmlBaseName.ga"
    diamId=$(cat "$diamIdFile" | grep "Extremities: " | cut -d " " -f 2 | cut -d "," -f  1)
    diamTime="$minIdTime"
    
    e2aceFile="$confDir/$xmlBaseName.e2ace"
    #e2aceId=$(cat "$e2aceFile" | grep "has won the election" | cut -d " " -f 1 | cut -d "@" -f 2)
   
    e2aceId=$(cat "$e2aceFile" | grep "has won the election" | cut -d ":" -f 1 | cut -d "@" -f 2)
    e2aceTime=$(cat "$e2aceFile" | grep "has won the election" | cut -d " " -f 7| cut -d "!" -f 1)

    #maxId=$(cat "$diamIdFile" | grep "Graph: " | cut -d " " -f 2)
    
    stats="individualStats.dat"
    
    echo -e "\tmin-id: $minId at $minIdTime (us)"
    echo -e "\tDiameter-id: $diamId at $diamTime (us)"
    echo -e "\te2ace-id: $e2aceId at $e2aceTime (us)"
    #echo -e "\tmax-id: $maxId"
    
    # to have identical configurations (to be fair with all tested protocols)
    clockModelOption="-C 3 $e2aceId $diamId $minId"
    
    # Tree Diameter
    variant="tree-diameter"
    parameters="-P $TREE -M $diamId $diamTime -L $winSize -F -S $period1MRTP -Q $period2MRTP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    # Tree E2ACE
    variant="tree-e2ace"
    parameters="-P $TREE -M $e2aceId $e2aceTime -L $winSize -F -S $period1MRTP -Q $period2MRTP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    # Convergecast Diameter
    variant="convergecast-diameter"
    parameters="-P $CONVERGECAST -M $diamId $diamTime -L $winSize -F -S $period1MRTP -Q $period2MRTP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    # Convergecast E2ACE
    variant="convergecast-e2ace"
    parameters="-P $CONVERGECAST -M $e2aceId $e2aceTime -L $winSize -F -S $period1MRTP -Q $period2MRTP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    #MRTP Diameter
    variant="mrtp-diameter"
    parameters="-P $MRTP -M $diamId $diamTime -L $winSize -F -S $period1MRTP -Q $period2MRTP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    #MRTP Diameter RTT
    #variant="mrtp-diameter-rtt"
    #parameters="-P $MRTP -M $diamId $diamTime -L $winSize -F -S $period1MRTP -Q $period2MRTP -T"
    #executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    #MRTP E2ACE NO REG
    variant="mrtp-e2ace-noreg"
    parameters="-P $MRTP -M $e2aceId $e2aceTime -F -S $period1MRTP -Q $period2MRTP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    #MRTP E2ACE
    variant="mrtp-e2ace"
    parameters="-P $MRTP -M $e2aceId $e2aceTime -L $winSize -F -S $period1MRTP -Q $period2MRTP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    #MRTP E2ACE RTT
    #variant="mrtp-e2ace-rtt"
    #parameters="-P $MRTP -M $e2aceId $e2aceTime -L $winSize -F -S $period1MRTP -Q $period2MRTP -T"
    #executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    # TPSN = with no regression linear, rtt, no convergecast
    #variant="tpsn-e2ace"
    #parameters="-P $TPSN -M $e2aceId $e2aceTime -S $period1TPSN -Q $period1TPSN -T"
    #executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    # TPSN MLE CENTER
    variant="tpsn-e2ace-mle"
    parameters="-P $TPSNMLE -M $e2aceId $e2aceTime -S $period1TPSN -Q $period1TPSN -T"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    # TPSN MLE DIAM
    variant="tpsn-diameter-mle"
    parameters="-P $TPSNMLE -M $diamId $diamTime -S $period1TPSN -Q $period1TPSN -T"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    #AD
    #variant="ad-rtt"
    #parameters="-P $AD -S $period1AD -T"
    #executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    #AD
    #variant="ad"
    #parameters="-P $AD -S $period1AD"
    #executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    #ATS
    variant="ats"
    parameters="-P $ModifiedATS -S $period1ATS"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    # FTSP
    variant="ftsp-diameter"
    parameters="-P $FTSP -S $period1FTSP -L $winSizeFTSP"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"

    # PulseSync
    variant="PulseSync"
    parameters="-P $PULSESYNC -S $period1PULSESYNC -L $winSizePULSESYNC"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    # WMTS
    variant="wmts"
    parameters="-P $WMTS -S $period1WMTS"
    executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
    
    # GTSP
    #variant="gtsp"
    #parameters="-P $GTSP -S $period1GTSP"
    #executeAndCopyFiles "$xml" "$variant" "$subResDir" "$parameters"
  
    echo "============"
done
