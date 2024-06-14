#!/bin/sh

DB=DB_DOMPIWEB
HOSTNAME="https://witchblade.com.ar"
UPLOAD_FORM="dpc/upload_client_config.php"

FECHA=`date +%y%m%d%H%M%S`
BACKUP_PATH="/var/gmonitor/backup"
INFILE=/usr/local/bin/make_dump.sql

echo "Obteniendo sistema..."
SYSTEM_KEY=`echo "SELECT System_Key FROM DB_DOMPIWEB.TB_DOM_CONFIG ORDER BY Id DESC LIMIT 1;" | /usr/bin/mysql -D "${DB}" -N -r`
echo "        Sistema: ${SYSTEM_KEY}"
FILE="backup-mysql-${DB}-${SYSTEM_KEY}-${FECHA}.sql"
OUTFILE="${BACKUP_PATH}/${FILE}"

mkdir -p $BACKUP_PATH

echo "Optimizando Base de Datos ${DB} ..."
/usr/bin/mysqloptimize $DB >/dev/null

echo "Generando ${OUTFILE} ..."

echo "-- #### Backup creado el ${FECHA}" > "${OUTFILE}"
echo "-- ####" >> "${OUTFILE}"
echo "-- ####CREATE DATABASE ${DB};" >> "${OUTFILE}"
echo "-- ####CREATE USER 'dompi_web'@'%' IDENTIFIED BY 'dompi_web';" >> "${OUTFILE}"
echo "-- ####GRANT SELECT, INSERT, UPDATE, DELETE ON DB_DOMPIWEB.* TO 'dompi_web'@'%' WITH GRANT OPTION;" >> "${OUTFILE}"
echo "-- ####FLUSH PRIVILEGES;" >> "${OUTFILE}"
echo "-- ####" >> "${OUTFILE}"
echo "USE ${DB};" >> "${OUTFILE}"
echo "-- ####" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_AUTO;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_AT;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_EVENT;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_CAMARA;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ALARM_SALIDA;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ALARM_ZONA;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ALARM_PARTICION;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_FLAG;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_GROUP;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ASSIGN;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_GRUPO_VISUAL;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_PERIF;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_USER;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_CONFIG;" >> "${OUTFILE}"
echo "-- ####" >> "${OUTFILE}"

/usr/bin/mysql -D "${DB}" -N -r < "${INFILE}" >> "${OUTFILE}"
sed -i 's/\t//g' "${OUTFILE}"
sed -i 's/NULL//g' "${OUTFILE}"
sed -i 's/,,/,NULL,/g' "${OUTFILE}"

echo "Subiendo backup a ${HOSTNAME}..."
/usr/bin/curl --insecure -F "file=@${OUTFILE};filename=${FILE}" "${HOSTNAME}/${UPLOAD_FORM}" >/dev/null
