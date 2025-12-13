#!/bin/sh

USER=$1

monitor ()
{
	ls -l /proc | awk '{ print $3 " " $9 }' | grep ^$USER | while read OWNER DIR; do

		if [ -f /proc/$DIR/status ]; then
			FECHA=`date "+%Y%m%d"`
			HORA=`date "+%H:%M:%S"`
			PROCESO=`cat /proc/$DIR/status | grep "^Name:" | awk '{ print $2 }'`

			if [ "X${PROCESO}" != "X" ]; then
				OUTFILE="/var/log/monitor/mon-${FECHA}-${PROCESO}-${DIR}.log"
				echo "==============================" >> "${OUTFILE}"
				echo "== ${HORA}" >> "${OUTFILE}"
				cat /proc/$DIR/status | egrep "VmPeak|VmHWM|VmData" >> "${OUTFILE}"
			fi
		fi
	done
}

mkdir -p /var/log/monitor

yes | while read yes; do
	monitor
	sleep 1
done

