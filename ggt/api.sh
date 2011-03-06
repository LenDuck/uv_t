#!/bin/sh
if [ $# -eq 1 ]
then echo "Verbose mode on"
fi

KEY="AIzaSyASdf7nYYdQfP_WVLoF0ycmG6ZeogBT-VM"
REQ=`./formrequest ${KEY}`


if [ $# -eq 1 ]
then echo ${REQ}
fi

#Files used for storage
RESP='.response'
TRANS='.translation'
ERR=/dev/null

#Retrieve data, if not verbose ditch stderr
if [ $# -eq 1 ]
then `curl -k ${REQ} > ${RESP}`
else `curl -k ${REQ} 2>${ERR} 1> ${RESP}`
fi

#If verbose print the raw response
if [ $# -eq 1 ]
then cat $RESP
fi

`cat ${RESP} | ./soapscan > ${TRANS}`

echo Translation:
cat ${TRANS}
echo

#delete the temporary files
rm ${RESP}
rm ${TRANS}
