#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ `mysql_config --cflags` -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lgmc -lssl -lcrypto `mysql_config --libs`
DEFINE=$(GENERAL_DEFINE)
CC=g++
OBJECTS=$(OBJ)/dompi_netcomm.o $(OBJ)/dom32iowifi.o $(OBJ)/strfunc.o $(OBJ)/config.o $(OBJ)/queue.o $(OBJ)/rbpiio.o $(OBJ)/cmydb.o $(OBJ)/gevent.o
OUTPUT=$(PROG)/$(EXE)
EXE=dompi_netcomm

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: $(OUTPUT)

$(OBJ)/dompi_netcomm.o: dompi_netcomm.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c dompi_netcomm.cc -o $(OBJ)/dompi_netcomm.o
$(OBJ)/queue.o: ../../Common/queue.c
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/queue.c -o $(OBJ)/queue.o
$(OBJ)/config.o: ../../Common/config.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/config.cc -o $(OBJ)/config.o
$(OBJ)/strfunc.o: ../../Common/strfunc.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/strfunc.cc -o $(OBJ)/strfunc.o
$(OBJ)/dom32iowifi.o: ../../Common/dom32iowifi.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/dom32iowifi.cc -o $(OBJ)/dom32iowifi.o
$(OBJ)/gevent.o: ../../Common/gevent.cc ../../Common/gevent.h
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/gevent.cc -o $(OBJ)/gevent.o
$(OBJ)/rbpiio.o: ../../Common/rbpiio.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/rbpiio.cc -o $(OBJ)/rbpiio.o
$(OBJ)/cmydb.o: ../../Common/cmydb.cc ../../Common/cmydb.h
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/cmydb.cc -o $(OBJ)/cmydb.o

$(OUTPUT): $(OBJECTS)
	$(MKDIR) $(PROG)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RMR) $(MACHINE)

install:
	$(CHMOD) +x install_server.sh
	$(CHMOD) +x update-tables.sh
	./install_server.sh $(EXE) $(PROG) $(INST_SBINDIR) $(INST_VARDIR)

uninstall:
	$(RM) $(INST_SBINDIR)/$(EXE)
