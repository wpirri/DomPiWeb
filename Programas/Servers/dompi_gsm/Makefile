#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lgmc -lssl -lcrypto
DEFINE=$(GENERAL_DEFINE)
CC=g++
OBJECTS=$(OBJ)/dompi_gsm.o $(OBJ)/config.o $(OBJ)/modulo_gsm.o $(OBJ)/serial.o
OUTPUT=$(PROG)/$(EXE)
EXE=dompi_gsm

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: $(OUTPUT)

$(OBJ)/dompi_gsm.o: dompi_gsm.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c dompi_gsm.cc -o $(OBJ)/dompi_gsm.o
$(OBJ)/config.o: ../../Common/config.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/config.cc -o $(OBJ)/config.o
$(OBJ)/modulo_gsm.o: modulo_gsm.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c modulo_gsm.cc -o $(OBJ)/modulo_gsm.o
$(OBJ)/serial.o: ../../Common/serial.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/serial.cc -o $(OBJ)/serial.o

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
