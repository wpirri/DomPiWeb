#!/bin/sh

SU_USER="gmonitor"
SYTEM_HOME=/home/$SU_USER
DOCUMENT_ROOT=/var/www/html
CGI_ROOT=/usr/lib/cgi-bin/
FILE_NAME=gmonitor_dompiweb_update.tar.gz
UPDATE_FILE="/home/${SU_USER}/${FILE_NAME}"
OLD_UPDATE_FILE="/home/${SU_USER}/old_gmonitor_dompiweb_update.tar.gz"

CHECK_PATH=/var/www/html/upload
MOVE_TO=$SYTEM_HOME
FIRMWARE_PATH=/var/www/html/download

check_for_updates_daemon()
{
	if [ -f $CHECK_PATH/$FILE_NAME ]; then
		sleep 5
		mv $CHECK_PATH/$FILE_NAME $MOVE_TO/
		logger "[DOMPIWEB] Aplicando actualizacion y reiniciando"
		/etc/init.d/gmond stop
              
		if [ -f ${UPDATE_FILE} ]; then
			logger "[DOMPIWEB] Actualizando sistema con: ${UPDATE_FILE}..."
			cd /
			tar xvzf ${UPDATE_FILE}
			cp -uvr $SYTEM_HOME/html/* $DOCUMENT_ROOT
			cp -uvr $SYTEM_HOME/cgi/* $CGI_ROOT
			rm -rv $SYTEM_HOME/*
		fi

		/etc/init.d/gmond start
	fi

    ls $CHECK_PATH/*.hex 2>/dev/null | while read FILENAME; do
        mv $FILENAME $FIRMWARE_PATH/
	done

	rm -f $CHECK_PATH/*
}

check_for_updates_loop()
{
	while true
       	do
		check_for_updates_daemon
		sleep 5
	done
}

check_for_updates_loop &






