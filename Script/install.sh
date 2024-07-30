#!/bin/sh

#
# Este Script se ejecuta después de descomprimir el archivo
# gmonitor_dompiweb_install.tar.gz 
#

GMON_USER=gmonitor
SYTEM_HOME=/home/$GMON_USER
SYTEM_VAR=/var/$GMON_USER
SYTEM_LIB=/var/lib/$GMON_USER
SYTEM_LOG=/var/log/$GMON_USER
SQL=/usr/bin/mysql

DOCUMENT_ROOT=/var/www/html
CGI_ROOT=/usr/lib/cgi-bin/


if [ ! -x $SQL ]; then
    echo "No se encuentra mysql"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

if [ ! -x /usr/sbin/xinetd ]; then
    echo "No se encuentra xinetd"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

if [ ! -x /usr/sbin/apache2 ]; then
    echo "No se encuentra apache2"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

if [ ! -x /usr/bin/php ]; then
    echo "No se encuentra php"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

if [ ! -x /usr/share/doc/libapache2-mod-php ]; then
    echo "No se encuentra libapache2-mod-php"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

x=`find /usr/lib -name libcjson.so`
if [ "X${x}" = "X" ]; then
    echo "No se encuentra libcjson.so"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

x=`find /usr/lib -name libssl.so`
if [ "X${x}" = "X" ]; then
    echo "No se encuentra libssl.so"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

if [ ! -x /usr/bin/curl ]; then
    echo "No se encuentra curl"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

if [ ! -x /usr/bin/openssl ]; then
    echo "No se encuentra curl"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php default-mysql-server libcjson1 curl openssl"
    exit 1
fi

echo "Agregando usuario ${GMON_USER}"
useradd -d $SYTEM_LIB $GMON_USER
passwd -d $GMON_USER
chown -R $GMON_USER $SYTEM_HOME

echo "Creando directorios..."
# Creo los directorios necesarios
mkdir -v -p $SYTEM_VAR
chown -v $GMON_USER: $SYTEM_VAR
mkdir -v -p $SYTEM_LIB/saf
chown -v $GMON_USER: $SYTEM_LIB/saf
mkdir -v -p $SYTEM_LIB
chown -v $GMON_USER: $SYTEM_LIB
mkdir -v -p $SYTEM_LOG
chown -v $GMON_USER: $SYTEM_LOG
mkdir -v -p $DOCUMENT_ROOT
# Directorio para upload / download
mkdir -v -p $DOCUMENT_ROOT/upload
chmod 0777 $DOCUMENT_ROOT/upload
mkdir -v -p $DOCUMENT_ROOT/download
chmod 0777 $DOCUMENT_ROOT/download

echo "Agregando la configuracion de gmonitor y DomPiWeb..."
# Agrego los la configuración de gmonitor y DomPiWeb
cp -v $SYTEM_HOME/funcion.tab $SYTEM_LIB
cp -v $SYTEM_HOME/funcion_parametro.tab $SYTEM_LIB
cp -v $SYTEM_HOME/server.tab $SYTEM_LIB
cp -v $SYTEM_HOME/server_parametro.tab $SYTEM_LIB
chown -v -R $GMON_USER: $SYTEM_LIB
cp -v $SYTEM_HOME/dompiweb.config /etc/

echo "Creando Base de datos..."
# Creo la base de datos
$SQL < $SYTEM_HOME/create.sql
echo "Inicializando Base de datos..."
$SQL < $SYTEM_HOME/init.sql

echo "Agregando script de arranque..."
# Agrego el script de arranque
systemctl daemon-reload
systemctl enable gmonitor.service
#systemctl start gmonitor.service
rm /etc/init.d/gmond
ln -s /usr/local/sbin/gmond /etc/init.d/gmond

echo "Generando arbol web..."
# Copio el arbol web y cgi
cp -rv $SYTEM_HOME/html/* $DOCUMENT_ROOT
cp -rv $SYTEM_HOME/cgi/* $CGI_ROOT

echo "Configurando y reiniciando xinetd..."
# Agregando servicio a /etc/services
x=`grep gmonitor /etc/services`
if [ "X${x}" = "X" ]; then
    echo "gmonitor        5533/tcp        # Gnu-Monitor" >> /etc/services
fi
service xinetd restart

echo "Configurando y reiniciando httpd..."
# Agregando CGI al Apache
a2enmod cgi
service apache2 restart

# Parametros TCP
echo "Configurando TCP..."
/sbin/sysctl -w net.ipv4.tcp_keepalive_intvl=75
/sbin/sysctl -w net.ipv4.tcp_keepalive_probes=8
/sbin/sysctl -w net.ipv4.tcp_keepalive_time=75

echo "# Agregado por DomPiWeb" >> /etc/sysctl.conf
echo "net.ipv4.tcp_keepalive_intvl=1" >> /etc/sysctl.conf
echo "net.ipv4.tcp_keepalive_probes=10" >> /etc/sysctl.conf
echo "net.ipv4.tcp_keepalive_time=80" >> /etc/sysctl.conf
echo "=================================================================="
echo
# Mensajes finales
echo "Ejecutar:"
echo "  rm -rv ${SYTEM_HOME}/*" para limpiar el home fr gmonitor
echo
echo "Modificar en php.ini y luego reiniciar Apache:"
echo "   upload_max_filesize=10M"
echo "   post_max_size=11M"
echo
