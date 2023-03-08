#!/usr/bin/make -f

INCLUDE=-I../../Common/ `mysql_config --cflags`
CXXFLAGS=-g -Wall -Wextra -MMD -O2
NO_LIBS=-lsqlite3
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lssl -lcrypto `mysql_config --libs`
OBJECTS=dompi_server.o cmydb.o config.o strfunc.o gevent.o
OUTPUT=dompi_server
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_server

dompi_server.o: dompi_server.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_server.cc

gevent.o: ../../Common/gevent.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/gevent.cc
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

dompi_server: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	chmod +x install_server.sh
	./install_server.sh $(OUTPUT)

uninstall:
	$(RM) /usr/local/sbin/$(OUTPUT)
