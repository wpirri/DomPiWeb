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
DBPATH=/var/lib/DomPiWeb
DATABASE=.DomPiWebDB.sqll3
SQL=/usr/bin/sqlite3

DOCUMENT_ROOT=/var/www/html
CGI_ROOT=/usr/lib/cgi-bin/


if [ ! -x $SQL ]; then
    echo "No se encuentra sqlite3"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php sqlite3 libcjson-dev"
    exit 1
fi

if [ ! -x /usr/sbin/xinetd ]; then
    echo "No se encuentra xinetd"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php sqlite3 libcjson-dev"
    exit 1
fi

if [ ! -x /usr/sbin/apache2 ]; then
    echo "No se encuentra apache2"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php sqlite3 libcjson-dev"
    exit 1
fi

if [ ! -x /usr/bin/php ]; then
    echo "No se encuentra php"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php sqlite3 libcjson-dev"
    exit 1
fi

if [ ! -x /usr/share/doc/libapache2-mod-php ]; then
    echo "No se encuentra libapache2-mod-php"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php sqlite3 libcjson-dev"
    exit 1
fi

if [ ! -x /usr/lib/x86_64-linux-gnu/libcjson.so && ! -L /usr/lib/x86_64-linux-gnu/libcjson.so ]; then
    echo "No se encuentra libcjson.so"
    echo "Se debe instalar xinetd apache2 php libapache2-mod-php sqlite3 libcjson-dev"
    exit 1
fi

echo "Construyendo los links de las librerias..."
# Reconstruyo los links
ln -vs /usr/lib/libgmc.so.0.0.0 /usr/lib/libgmc.so
ln -vs /usr/lib/libgmc.so.0.0.0 /usr/lib/libgmc.so.0 
ln -vs /usr/lib/libgmq.so.0.0.0 /usr/lib/libgmq.so
ln -vs /usr/lib/libgmq.so.0.0.0 /usr/lib/libgmq.so.0
ln -vs /usr/lib/libgmqw.so.0.0.0 /usr/lib/libgmqw.so
ln -vs /usr/lib/libgmqw.so.0.0.0 /usr/lib/libgmqw.so.0
ln -vs /usr/lib/libgmshared.so.0.0.0 /usr/lib/libgmshared.so
ln -vs /usr/lib/libgmshared.so.0.0.0 /usr/lib/libgmshared.so.0

echo "Creando directorios..."
# Creo los directorios necesarios
mkdir -v -p $SYTEM_VAR
chown -v $GMON_USER: $SYTEM_VAR
mkdir -v -p $SYTEM_LIB
chown -v $GMON_USER: $SYTEM_LIB
mkdir -v -p $SYTEM_LOG
chown -v $GMON_USER: $SYTEM_LOG
mkdir -v -p $DOCUMENT_ROOT

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
mkdir -v -p $DBPATH
chown -v -R gmonitor: $DBPATH
su -c "$SQL < $SYTEM_HOME/create.sql ${DBPATH}/${DATABASE}" $GMON_USER
echo "Inicializando Base de datos..."
su -c "$SQL < $SYTEM_HOME/init.sql $DBPATH/$DATABASE" $GMON_USER
chown -v -R gmonitor: $DBPATH/$DATABASE
chmod -v 0666 $DBPATH/$DATABASE

echo "Agregando script de aranque..."
# Agrego el script de arranque
cp -v $SYTEM_HOME/gmond /etc/init.d

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

# Limpiesa final
rm -rv $SYTEM_HOME/*
