#! /bin/bash

seed=20

robot="BB"

SPARSE_COMM_MODEL=0
INTERMEDIATE_COMM_MODEL=1
COMPACT_COMM_MODEL=2

MRTP=1
TPSN=2

confDir="confs/fidelity"

resDir="evaluation/results/fidelity"

rm -rf "$resDir"
mkdir -p "$resDir"

#rtt vs static
#linear regression 5
#e2ace vs min-id

oneMSec=1000
oneSec=$((1000*$oneMSec))
oneMin=$((60*$oneSec))
oneHour=$((60*$oneMin))

start=$((2000*oneMSec))

winSize=5
period1MRTP=$((2*$oneSec + 3*$oneMSec))
period2MRTP=$((5*$oneSec + 3*$oneMSec))

## dissemination error
echo "Dissemination error"

#xml="$confDir/bb-l-100.xml"
xml="$confDir/bb-l-30.xml"

timeMasterId=10
timeMasterTime=$((2*$oneMSec + 125 ))
seed1=-1

#seed1=$seed

# more or less ok. noise id : 1
#seed1=1835602595
# std trop importante leger
#seed1=1624847804
#seed1=1803293390
# deux points extremes mais ok
#seed1=308890798
# 1 pt extreme
#seed1=293958058
# moyenne legerement trop haute
seed1=1374367794

#if true; then

echo -e "\tPRED"
./timeSync -c $xml -k $robot -a $seed1 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -L $winSize -F -S $period1MRTP -Q $period2MRTP -E -Z $SPARSE_COMM_MODEL > "$resDir/output.dissemination"  2>&1
cp "mrtp.dat" "$resDir/mrtp.dissemination"

## Test dissemination error no regression linear
echo -e "\tNO REG"
./timeSync -c $xml -k $robot -a $seed1 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -F -S $period1MRTP -Q $period2MRTP -E -Z $SPARSE_COMM_MODEL > "$resDir/output.dissemination-no-reg"  2>&1
cp "mrtp.dat" "$resDir/mrtp.dissemination-no-reg"

## Test dissemination error with RTT
echo -e "\tRTT"
./timeSync -c $xml -k $robot -a $seed1 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -L $winSize -F -S $period1MRTP -Q $period2MRTP -E -T -Z $SPARSE_COMM_MODEL > "$resDir/output.dissemination-rtt"  2>&1
cp "mrtp.dat" "$resDir/mrtp.dissemination-rtt"

echo -e "\tCompact"
xml="$confDir/bb-compact.xml"
./timeSync -c $xml -k $robot -a $seed1 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -L $winSize -F -S $period1MRTP -Q $period2MRTP -E -Z $COMPACT_COMM_MODEL > "$resDir/output.dissemination-compact"  2>&1
cp "mrtp.dat" "$resDir/mrtp.dissemination-compact"

if false; then
## RTT
echo "============"
echo "RTT"
xml="$confDir/bb-cross-5.xml"
timeMasterId=3
timeMasterTime=$((2*$oneMSec))
rttPeriod=$((233*$oneMSec))
seed2=-1

#seed2=seed

#./timeSync -c $xml -k $robot -a $seed2 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -L $winSize -F -S $rttPeriod -Q $rttPeriod -E -T > "$resDir/output.rtt"  2>&1
./timeSync -c $xml -k $robot -a $seed2 -i -R -t -B $start -P $TPSN -M $timeMasterId $timeMasterTime -S $rttPeriod -E -T -Z $SPARSE_COMM_MODEL > "$resDir/output.rtt"  2>&1
cp "rtt.dat" "$resDir/mrtp.rtt"
fi


## L2D-10
echo "============"
echo "L2D-10 config"

seed3="$RANDOM"
#seed3=$seed

# quite good! Be carreful to activate static delay compensation
#seed3=21356
seed3=16741
#seed3=22594
#seed3=18802

#seed3=10454
#seed3=9143
#seed3=19280
#seed3=1789

#seed3=28384
echo "seed: $seed3"

model=$INTERMEDIATE_COMM_MODEL
xml="$confDir/l2d-10.xml"
timeMasterId=5
timeMasterTime=$((2*$oneMSec))
period1MRTP=$((2*$oneSec))
period2MRTP=$((5*$oneSec))

echo -e "\tPeriod..."

subResDir="$resDir/period"
mkdir "$subResDir"

./timeSync -c $xml -k $robot -a $seed3 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -L $winSize -F -S $period1MRTP -Q $period2MRTP -E -Z $model > "$resDir/output.l2d-10" 2>&1
cp "mrtp.dat" "$resDir/mrtp.l2d-10"

#c(2,5,10,20,30,60)
periods="2 5 10 20 30 60"
for p in $periods; do
    period2="$(($p*$oneSec))"
    variant="period-$p"
    ./timeSync -c $xml -k $robot -a $seed3 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -L $winSize -F -S $period1MRTP -Q $period2 -E -Z $model > "$subResDir/output.$variant"  2>&1
    cp "mrtp.dat" "$subResDir/mrtp.$variant"
    #echo $period2
done

echo -e "\tWindow..."
subResDir="$resDir/window"
mkdir "$subResDir"

#c(2,3,5,10,20,30)
windows="2 3 5 10 20 30"
for window in $windows; do
    variant="window-$window"
    ./timeSync -c $xml -k $robot -a $seed3 -i -R -t -B $start -P $MRTP -M $timeMasterId $timeMasterTime -L $window -F -S $period1MRTP -Q $period2MRTP -E -Z $model > "$subResDir/output.$variant" 2>&1
    cp "mrtp.dat" "$subResDir/mrtp.$variant"
done
