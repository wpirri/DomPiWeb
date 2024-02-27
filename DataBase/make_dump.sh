#!/bin/sh

HOST=192.168.10.31
DB=DB_DOMPIWEB
USER=dompi_web
PASS=dompi_web

FECHA=`date +%y%m%d%H%M%S`

INFILE=/usr/local/bin/make_dump.sql
OUTFILE=$HOME/dump-"${DB}"-"${FECHA}".sql

echo "Generando ${OUTFILE} ..."

echo "#### Backup creado el ${FECHA}" > "${OUTFILE}"
echo "####" > "${OUTFILE}"
echo "#CREATE DATABASE ${DB};" >> "${OUTFILE}"
echo "#CREATE USER 'dompi_web'@'%' IDENTIFIED BY 'dompi_web';" > "${OUTFILE}"
echo "#GRANT SELECT, INSERT, UPDATE, DELETE ON DB_DOMPIWEB.* TO 'dompi_web'@'%' WITH GRANT OPTION;" > "${OUTFILE}"
echo "#FLUSH PRIVILEGES;" > "${OUTFILE}"
echo "####" > "${OUTFILE}"
echo "#USE ${DB};" >> "${OUTFILE}"
echo "####" > "${OUTFILE}"
echo "DELETE FROM TB_DOM_CAMARA;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ALARM_SALIDA;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ALARM_ZONA;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ALARM_PARTICION;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_AUTO;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_AT;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_EVENT;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_FUNCTION;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_FLAG;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_GROUP;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_ASSIGN;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_GRUPO_VISUAL;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_PERIF;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_USER;" >> "${OUTFILE}"
echo "DELETE FROM TB_DOM_CONFIG;" >> "${OUTFILE}"
echo "####" > "${OUTFILE}"

#echo "mysql -h ${HOST} -u ${USER} -p${PASS} -D ${DB} -N -r < ${INFILE} > ${OUTFILE}"
mysql -h "${HOST}" -u "${USER}" -p"${PASS}" -D "${DB}" -N -r < "${INFILE}" >> "${OUTFILE}"
sed -i 's/\t//g' "${OUTFILE}"
sed -i 's/NULL//g' "${OUTFILE}"
sed -i 's/,,/,NULL,/g' "${OUTFILE}"
