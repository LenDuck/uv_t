#!/bin/bash


#Files used for storage
REQUESTFILE='.request'
RESP='.response'
TRANS='.translation'
ERR=/dev/null


#Make if not found, to ease the initial run
if [ ! -x "./soapscan" ] ; then
  echo File not executable, running make
  make
fi

if [ ! -x "./formrequest" ] ; then
  echo File not executable, running make
  make
fi

#Check for the --help argument, if found print and exit
if [ "$#" -eq "1" ] ; then
if [ "$1" == "--help" ] ; then 
echo "Usage:"
echo "$0 (From) (To) (Inputfile) (Keep_temp)"
echo "Items between () are optional but in fixed order"
echo "if keep_temp is 1 some temporary files are left for examination"
echo
exit
fi
fi

#The variable that decides to leave some temp files laying around
KEEPTEMP=$4

#Key needed by API
KEY="AIzaSyASdf7nYYdQfP_WVLoF0ycmG6ZeogBT-VM"

#Form the request, passing any arguments to it (it ignores the keeptemp)
REQ=`./formrequest ${KEY} $*`

#If needed store the request
if [ "${KEEPTEMP}" == "1" ] ; then
echo ${REQ} > ${REQUESTFILE}
fi

#Request the translation, and parse it
#using files due to some weird vibe with newlines
`curl -k ${REQ} 2>${ERR} 1> ${RESP}`
`cat "${RESP}" | ./soapscan 1> ${TRANS}`

#Print the result
echo Translation:
cat ${TRANS}
echo

#And clean up unless you want the temp files
if [ "${KEEPTEMP}" == "1" ] ; then
  echo 'Temporary files:'
  echo ${REQESTFILE}
  echo ${RESP}
  echo ${TRANS}
else
  #delete the temporary files
  rm ${RESP}
  rm ${TRANS}
fi
