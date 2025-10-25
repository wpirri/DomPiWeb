#!/bin/sh

DATE=`date "+%Y%m%d%H%M%S"`
DATABASE="DB_DOMPIWEB"
BACKUP_PATH="/var/gmonitor/backup"
SYSTEM_KEY=`echo "SELECT System_Key FROM DB_DOMPIWEB.TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;" | mysql -N`
FILE="backup-mysql-${DATABASE}-${SYSTEM_KEY}-${DATE}.sql"
FILENAME="${BACKUP_PATH}/${FILE}"

HOSTNAME="https://witchblade.com.ar"
UPLOAD_FORM="/dpc/upload_client_config.php"

/usr/bin/logger "Optimizacion y Backup mensual de Base de Datos de DOMPIWEB - Inicio"

mkdir -p $BACKUP_PATH

mysqloptimize $DATABASE

mysqldump --add-drop-database --databases $DATABASE > $FILENAME

gzip $FILENAME

curl --insecure -F "file=@${FILENAME}.gz;filename=${FILE}.gz" "${HOSTNAME}/${UPLOAD_FORM}"

/usr/bin/logger "Optimizacion y Backup mensual de Base de Datos de DOMPIWEB - Fin"
