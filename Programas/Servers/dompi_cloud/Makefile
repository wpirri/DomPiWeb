#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ `mysql_config --cflags` -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
NO_LIBS=-lsqlite3
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lssl -lcrypto `mysql_config --libs`
OBJECTS=dompi_cloud.o strfunc.o config.o cmydb.o gevent.o
OUTPUT=dompi_cloud
DEFINE=-D __NO__DEBUG__ -D __NO_DEBUG_TCP
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_cloud

dompi_cloud.o: dompi_cloud.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c dompi_cloud.cc

config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/strfunc.cc
#cpgdb.o: ../../Common/cpgdb.cc ../../Common/cpgdb.h
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/cpgdb.cc
#csqlite.o: ../../Common/csqlite.cc
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/csqlite.cc
cmydb.o: ../../Common/cmydb.cc ../../Common/cmydb.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/cmydb.cc
gevent.o: ../../Common/gevent.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/gevent.cc

dompi_cloud: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	sed -i 's/\r//g' conf/*
	$(CHMOD) +x install_server.sh
	$(CHMOD) +x update-tables.sh
	./install_server.sh $(INST_SBINDIR) $(OUTPUT) $(INST_VARDIR)

uninstall:
	$(RM) $(INST_SBINDIR)/$(OUTPUT)
