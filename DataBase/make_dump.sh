#!/bin/sh

HOST=127.0.0.1
DB=DB_DOMPIWEB
USER=dompi_web
PASS=dompi_web

FECHA=`date +%y%m%d%H%M%S`

INFILE=make_dump.sql
OUTFILE=dump-"${DB}"-"${FECHA}".sql

#echo "mysql -h ${HOST} -u ${USER} -p${PASS} -D ${DB} -N -r < ${INFILE} > ${OUTFILE}"
mysql -h "${HOST}" -u "${USER}" -p"${PASS}" -D "${DB}" -N -r < "${INFILE}" > "${OUTFILE}"
sed -i 's/\t//g' "${OUTFILE}"
sed -i 's/NULL//g' "${OUTFILE}"
sed -i 's/,,/,NULL,/g' "${OUTFILE}"
