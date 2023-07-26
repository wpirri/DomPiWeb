
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

BIN_MONITOR=\"$(INST_BINDIR)/gmt\"
MONITOR_CONFIG_PATH=\"$(INST_VARDIR)\"
LOG_FILES=\"$(INST_LOGDIR)\"
SAF_FILES=\"$(INST_VARDIR)/saf\"

GM_CONFIG_KEY=0x131313
GM_COMM_MSG_LEN=10240
MAX_SERVERS=255
MAX_SERVICES=4096

CP=cp
CP_UVA=cp -uva
RM=rm -f
RMR=rm -rf
MKDIR=mkdir -p
CHMOD=chmod
CHOWN=chown
