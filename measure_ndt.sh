#!/bin/bash

######### 	measure_ndt.sh	###########
#
#	Executes the NDT test provided by measurement kit.
#
#	Recieves the probeid as argument.

#!/bin/bash 
# Absolute path to this script, e.g. /home/user/bin/foo.sh
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in, thus /home/user/bin
SCRIPTPATH=$(dirname "$SCRIPT")
PROBEID=$(cat $1)

cd $SCRIPTPATH

# Executes the NDT test and sends the report.
./measurement_kit -a probeid=$PROBEID -c httpo://vowqsydjizhxdr67.onion ndt

# Removes all njson created.
#rm *.njson
