#!/bin/sh

HOST=192.168.10.31
DB=DB_DOMPIWEB
USER=dompi_web
PASS=dompi_web

FECHA=`date +%y%m%d%H%M%S`

INFILE=make_dump.sql
OUTFILE=dump-"${DB}"-"${FECHA}".sql

echo "use ${DB};" > "${OUTFILE}"

echo "delete from TB_DOM_ALARM_ZONA;" >> "${OUTFILE}"
echo "delete from TB_DOM_ALARM_SALIDA;" >> "${OUTFILE}"
echo "delete from TB_DOM_ALARM_PARTICION;" >> "${OUTFILE}"
echo "delete from TB_DOM_AUTO;" >> "${OUTFILE}"
echo "delete from TB_DOM_AT;" >> "${OUTFILE}"
echo "delete from TB_DOM_EVENT;" >> "${OUTFILE}"
echo "delete from TB_DOM_FUNCTION;" >> "${OUTFILE}"
echo "delete from TB_DOM_FLAG;" >> "${OUTFILE}"
echo "delete from TB_DOM_GROUP;" >> "${OUTFILE}"
echo "delete from TB_DOM_ASSIGN;" >> "${OUTFILE}"
echo "delete from TB_DOM_GRUPO_VISUAL;" >> "${OUTFILE}"
echo "delete from TB_DOM_PERIF;" >> "${OUTFILE}"
echo "delete from TB_DOM_USER;" >> "${OUTFILE}"
echo "delete from TB_DOM_CONFIG;" >> "${OUTFILE}"

#echo "mysql -h ${HOST} -u ${USER} -p${PASS} -D ${DB} -N -r < ${INFILE} > ${OUTFILE}"
mysql -h "${HOST}" -u "${USER}" -p"${PASS}" -D "${DB}" -N -r < "${INFILE}" >> "${OUTFILE}"
sed -i 's/\t//g' "${OUTFILE}"
sed -i 's/NULL//g' "${OUTFILE}"
sed -i 's/,,/,NULL,/g' "${OUTFILE}"
