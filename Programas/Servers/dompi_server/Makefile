#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ `mysql_config --cflags` -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
NO_LIBS=-lsqlite3
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lssl -lcrypto `mysql_config --libs`
OBJECTS=dompi_server.o cmydb.o config.o strfunc.o gevent.o alarma.o
OUTPUT=dompi_server
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_server

dompi_server.o: dompi_server.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_server.cc

config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc
#cpgdb.o: ../../Common/cpgdb.cc ../../Common/cpgdb.h
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/cpgdb.cc
#csqlite.o: ../../Common/csqlite.cc
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/csqlite.cc
cmydb.o: ../../Common/cmydb.cc ../../Common/cmydb.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/cmydb.cc
gevent.o: ../../Common/gevent.cc ../../Common/gevent.h
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/gevent.cc

alarma.o: alarma.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c alarma.cc

dompi_server: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	$(CHMOD) +x install_server.sh
	./install_server.sh $(INST_SBINDIR) $(OUTPUT) $(INST_VARDIR)

uninstall:
	$(RM) $(INST_SBINDIR)/$(OUTPUT)
