#!/bin/bash

set -e

#DIALOG=Xdialog --default-no
DIALOG=kdialog

#ROOT_PATH=""
ROOT_PATH="/pirateradio/"

#
# compatible with obexpushd >= 0.6
#

MODE="$1"
echo "MODE: $MODE" 1>&2
FROM=""
NAME=""
SUB_PATH="${ROOT_PATH}./"
LENGTH="0"
MIMETYPE=""
while read LINE; do
	echo "${LINE}" 1>&2
	if ( test -z "${LINE}" ); then
		break
	fi
	TAG=$(echo "${LINE}" | cut -f 1 -d ":")
	VALUE=$(echo "${LINE}" | cut -f 2- -d " ")
	echo "TAG: $TAG VALUE: $VALUE" 1>&2

	case $TAG in
	From)   FROM="${VALUE}";;
	Name)   NAME="${VALUE}";;
	Path)   SUB_PATH="${ROOT_PATH}${VALUE}/";;
	Length) LENGTH="${VALUE}";;
	Type)   MIMETYPE="${VALUE}";;
	esac
done

echo "MODE: ${MODE}" 1>&2

case "${MODE}" in
put)
	FILE="${SUB_PATH}${NAME}"
	#echo "script: testing ${FILE}..." 1>&2
	#test -z "${FILE}" && exit 1
	#echo "script: testing for existence of ${FILE}..." 1>&2
	#test -e "${FILE}" && exit 1

	#tell obexpushd to go on
	echo "OK"

	echo "script: storing file on disk..." 1>&2
	cat > "${FILE}"
	echo "script: file saved to disk." 1>&2
	
	if [[ $NAME == "pirateradio.config" ]]
	then
		systemctl restart mpradio
	elif [[ $NAME == "cmd.txt" ]]
	then
		bash /pirateradio/$NAME
	fi

	
	#setfattr -n "user.mime_type" -v "${MIMETYPE}" "${FILE}"
	sleep 1
	echo "script: done" 1>&2
	;;

get)
	FILE=${SUB_PATH}${NAME}
	test -z "${FILE}" && exit 1
	test -f "${FILE}" || exit 1

	stat --printf="Length: %s\n" ${FILE}
	stat --format="%y" "${FILE}" | date -u +"Time: %Y%m%dT%H%M%SZ"

#	MIMETYPE=$(getfattr -n "user.mime_type" "${FILE}")
#	if [ "${MIMETYPE}" ]; then
#	    echo "Type: ${MIMETYPE}"
#	fi

	echo ""
	cat "${FILE}"
	;;

delete)
	FILE=${SUB_PATH}${NAME}
	exec rm -rf "${FILE}"
	;;

listdir)
	FILE=$(mktemp)
	obex-folder-listing ${SUB_PATH} >${FILE} 2>/dev/null
	stat --printf="Length: %s\n" ${FILE}
	echo ""
	cat ${FILE}
	rm -f ${FILE}
	;;

createdir)
	exec mkdir -p ${SUB_PATH}
	;;

capability)
	;;
esac
exit 0
