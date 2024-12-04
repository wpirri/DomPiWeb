#!/bin/sh

DATE=`date "+%Y%m%d%H%M%S"`
DATABASE="DB_DOMPIWEB"
BACKUP_PATH="/var/gmonitor/backup"
SYSTEM_KEY=`echo "SELECT System_Key FROM DB_DOMPIWEB.TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;" | mysql -N`
FILE="backup-mysql-${DATABASE}-${SYSTEM_KEY}-${DATE}.sql"
FILENAME="${BACKUP_PATH}/${FILE}"

HOSTNAME="https://witchblade.com.ar"
UPLOAD_FORM="/dpc/upload_client_config.php"

mkdir -p $BACKUP_PATH

mysqloptimize $DATABASE

mysqldump --add-drop-database --databases $DATABASE > $FILENAME

curl --insecure -F "file=@${FILENAME};filename=${FILE}" "${HOSTNAME}/${UPLOAD_FORM}"
