
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

DATABASE=DB_DOMPIWEB
SQL=/usr/bin/mysql

UPDATE_FILE_ARM=gmonitor_dompiweb_update_arm.tar.gz
INSTALL_FILE_ARM=gmonitor_dompiweb_install_arm.tar.gz
UPDATE_FILE_I386=gmonitor_dompiweb_update_i386.tar.gz
INSTALL_FILE_I386=gmonitor_dompiweb_install_i386.tar.gz

MACHINE=.tmp_$(shell uname -n)

OBJ=$(MACHINE)/obj
PROG=$(MACHINE)/exe
INST=$(MACHINE)/inst

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
#PROCESSOR-PARAMS=-mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4

# Modulos opcionales
# -DALARMA_INTEGRADA
# -DACTIVO_ACTIVO
MODULOS_INCLUIDOS=

GM_COMM_MSG_LEN=32768

GENERAL_DEFINE=-D __NO__DEBUG__ -D __NO_DEBUG_TCP -DGM_COMM_MSG_LEN=$(GM_COMM_MSG_LEN)
