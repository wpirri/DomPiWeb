#!/bin/sh

DATE=`date "+%Y%m%d%H%M%S"`
DATABASE="DB_DOMPIWEB"
BACKUP_PATH="/var/www/html/download"
SYSTEM_KEY=`echo "SELECT System_Key FROM DB_DOMPIWEB.TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;" | mysql -N`
FILE="backup-mysql-${DATABASE}-${SYSTEM_KEY}-${DATE}.sql"
FILENAME="${BACKUP_PATH}/${FILE}"

HOSTNAME="https://witchblade.com.ar"
UPLOAD_FORM="dpc/upload_client_config.php"

# Creo el directorio por si no existe
mkdir -p $BACKUP_PATH
chmod 0777 $BACKUP_PATH
# Aprovecho para hacer una optimización mensual de la base local
mysqloptimize $DATABASE
# Genero un vuelco de la base local
mysqldump --add-drop-database --databases $DATABASE > $FILENAME
# Envío el backup a la nube
curl --insecure -F "file=@${FILENAME};filename=${FILE}" "${HOSTNAME}/${UPLOAD_FORM}"
# Mantengo los backup por 6 meses
find $BACKUP_PATH -type f -mtime +180 -name "*.sql" -execdir rm -f '{}' \;
