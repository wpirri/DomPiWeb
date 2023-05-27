#!/bin/sh

HOST=192.168.10.31
DB=DB_DOMPIWEB2
USER=dompi_web2
PASS=dompi_web2

FECHA=`date +%y%m%d%H%M%S`

INFILE=make_dump.sql
OUTFILE=dump-"${DB}"-"${FECHA}".sql

#echo "mysql -h ${HOST} -u ${USER} -p${PASS} -D ${DB} -N -r < ${INFILE} > ${OUTFILE}"
mysql -h "${HOST}" -u "${USER}" -p"${PASS}" -D "${DB}" -N -r < "${INFILE}" > "${OUTFILE}"
sed -i 's/\t//g' "${OUTFILE}"
sed -i 's/NULL//g' "${OUTFILE}"
sed -i 's/,,/,NULL,/g' "${OUTFILE}"
