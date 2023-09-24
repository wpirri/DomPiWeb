#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ `mysql_config --cflags` -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lgmc -lssl -lcrypto `mysql_config --libs`
OBJECTS=dompi_netcomm.o dom32iowifi.o ctcp.o strfunc.o config.o queue.o rbpiio.o cmydb.o gevent.o
OUTPUT=dompi_netcomm
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_netcomm

dompi_netcomm.o: dompi_netcomm.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c dompi_netcomm.cc

queue.o: ../../Common/queue.c
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/queue.c
ctcp.o: ../../Common/ctcp.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/ctcp.cc
config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/strfunc.cc
dom32iowifi.o: ../../Common/dom32iowifi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/dom32iowifi.cc
gevent.o: ../../Common/gevent.cc ../../Common/gevent.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/gevent.cc
rbpiio.o: ../../Common/rbpiio.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/rbpiio.cc
#cpgdb.o: ../../Common/cpgdb.cc ../../Common/cpgdb.h
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/cpgdb.cc
#csqlite.o: ../../Common/csqlite.cc
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/csqlite.cc
cmydb.o: ../../Common/cmydb.cc ../../Common/cmydb.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/cmydb.cc

dompi_netcomm: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	$(CHMOD) +x install_server.sh
	$(CHMOD) +x update-tables.sh
	./install_server.sh $(INST_SBINDIR) $(OUTPUT) $(INST_VARDIR)

uninstall:
	$(RM) $(INST_SBINDIR)/$(OUTPUT)
