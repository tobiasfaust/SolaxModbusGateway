#!/bin/bash

#Parameter

REPOSITORYNAME="$1"     # PumpControl
SUBVERSION="$2"         # Unique ID -> GITHUB_RUN_NUMBER
STAGE="$3"              # PROD|PRE|DEV
BINARYPATH="$4"         # Path of binaryFiles
RELEASEPATH="$5"        # Path of Destination, BIN and JSON Files
RELEASEFILE="$6"        # Path of ReleaseFile, contains versionnumber
ARCH="$7"               # Archtitcture, ESP8266|ESP32
ARTIFACTPATH="$8"       # Path of all Artifacts

readonly NC='\033[0m' # No Color
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'

#
# Get env parameter with higher priority, which enables the script to run directly in a step
#
if [[ -n $ENV_BINARYPATH ]]; then BINARYPATH=$ENV_BINARYPATH; fi
if [[ -n $ENV_SUBVERSION ]]; then SUBVERSION=$ENV_SUBVERSION; fi
if [[ -n $ENV_STAGE ]]; then STAGE=$ENV_STAGE; fi
if [[ -n $ENV_REPOSITORYNAME ]]; then REPOSITORYNAME=$ENV_REPOSITORYNAME; fi
if [[ -n $ENV_RELEASEFILE ]]; then RELEASEFILE=$ENV_RELEASEFILE; fi
if [[ -n $ENV_ARCH ]]; then ARCH=$ENV_ARCH; fi
if [[ -n $ENV_RELEASEPATH ]]; then RELEASEPATH=$ENV_RELEASEPATH; fi
if [[ -n $ENV_ARTIFACTPATH ]]; then ARTIFACTPATH=$ENV_ARTIFACTPATH; fi

#
# Echo input parameter
#
echo -e "\n\n"$YELLOW"Echo input parameter"$NC
echo REPOSITORYNAME=$REPOSITORYNAME
echo SUBVERSION=$SUBVERSION
echo STAGE=$STAGE
echo BINARYPATH=$BINARYPATH
echo RELEASEPATH=$RELEASEPATH
echo ARCHITECTURE=$ARCH
echo RELEASEFILE=$RELEASEFILE
echo ARTIFACTPATH=$ARTIFACTPATH

if [[ ! -d $BINARYPATH ]]; then
  echo -e "\n\n"$RED"Binarypath $BINARYPATH not found\n"$NC
  exit
fi

if [[ ! -f $RELEASEFILE ]]; then
  echo  -e "\n\n"$RED"Releasefile $RELEASEFILE not found\n"$NC
  exit
fi

if [[ ! -d $RELEASEPATH ]]; then
  mkdir -p $RELEASEPATH
fi

if [[ ! -d $ARTIFACTPATH ]]; then
  mkdir -p $ARTIFACTPATH
fi

VERSION=`sed 's/[^0-9|.]//g' $RELEASEFILE`  # 2.4.2
NUMBER=`sed 's/[^0-9]//g' $RELEASEFILE`     # 242

VERSION1=`echo $VERSION | cut -d '.' -f1`   # 2
VERSION2=`echo $VERSION | cut -d '.' -f2`   # 4
VERSION3=`echo $VERSION | cut -d '.' -f3`   # 2

let NUMBER=$(printf "%d%d%d%d" $VERSION1 $VERSION2 $VERSION3 $SUBVERSION)

for FILE in `find $BINARYPATH/ -name firmware.bin` 
do

  FILENAME=${FILE%.*}
  FILEEXT=${FILE/*./}

  BINARYFILENAME=$(basename $FILENAME"."$ARCH".v"$VERSION"-"$SUBVERSION"."$STAGE)
  DOWNLOADURL="http://tfa-releases.s3-website.eu-central-1.amazonaws.com/"$REPOSITORYNAME"/"$BINARYFILENAME"."$FILEEXT

  JSON='      {
  "name":"Release '$VERSION'-'$STAGE'",
  "version":"'$VERSION'",
  "subversion":'$SUBVERSION',
  "number":'$NUMBER',
  "stage":"'$STAGE'",
  "arch":"'$ARCH'",
  "download-url":"'$DOWNLOADURL'"
      }'

  echo -e "\n\n"$GREEN"Echo json string"$NC
  echo $JSON

  echo $JSON > $RELEASEPATH/$BINARYFILENAME".json"
  cp $FILE $RELEASEPATH/$BINARYFILENAME"."$FILEEXT

done

# process the rest ob binaries into ARTIFACTPATH
for FILE in `find $BINARYPATH/ -name *.bin` 
do
  FILENAME=${FILE%.*}
  FILEEXT=${FILE/*./}

  BINARYFILENAME=$(basename $FILENAME"."$ARCH".v"$VERSION"-"$SUBVERSION"."$STAGE)
  cp $FILE $ARTIFACTPATH/$BINARYFILENAME"."$FILEEXT

done
