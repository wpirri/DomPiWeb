
# sed -i 's/include "/include </g' *
# sed -i 's/\.h"/\.h>/g' *

RUN_USER=gmonitor
RUN_HOME=/home/gmonitor
INST_USER=root
INST_LIBDIR=/usr/lib
INST_HEADDIR=/usr/local/include/gmonitor
INST_BINDIR=/usr/local/bin
INST_SBINDIR=/usr/local/sbin
INST_ETCDIR=/etc/gmonitor
INST_VARDIR=/var/lib/gmonitor
INST_INCDIR=/usr/local/include
INST_LOGDIR=/var/log/gmonitor
INST_CGIDIR=/usr/lib/cgi-bin
INST_HTMLDIR=/var/www/html

DBPATH=/var/lib/DomPiWeb
DATABASE=.DomPiWebDB.sqll3
SQL=/usr/bin/sqlite3

UPDATE_FILE=gmonitor_dompiweb_update.tar.gz
INSTALL_FILE=gmonitor_dompiweb_install.tar.gz

CP=cp
CP_UVA=cp -uva
RM=rm -f
RMR=rm -rf
MKDIR=mkdir -p
CHMOD=chmod
CHOWN=chown

# RBPi2 - -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4
# RBPi3 - -mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
# RBPi4 - -mcpu=cortex-a72 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits
PROCESSOR-PARAMS=-mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4

# Modulos opcionales
# -DALARMA_INTEGRADA
# -DACTIVO_ACTIVO
MODULOS_INCLUIDOS=