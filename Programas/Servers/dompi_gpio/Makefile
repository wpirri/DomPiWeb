#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lwiringPi -lssl -lcrypto
DEFINE=$(GENERAL_DEFINE)
CC=g++
OBJECTS=$(OBJ)/dompi_gpio.o $(OBJ)/dom32iopi.o $(OBJ)/config.o $(OBJ)/strfunc.o $(OBJ)/serial.o
OUTPUT=$(PROG)/$(EXE)
EXE=dompi_gpio

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: $(OUTPUT)

$(OBJ)/dompi_gpio.o: dompi_gpio.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE)  $(MODULOS_INCLUIDOS) -c dompi_gpio.cc -o $(OBJ)/dompi_gpio.o
$(OBJ)/config.o: ../../Common/config.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/config.cc -o $(OBJ)/config.o
$(OBJ)/strfunc.o: ../../Common/strfunc.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/strfunc.cc -o $(OBJ)/strfunc.o
$(OBJ)/dom32iopi.o: ../../Common/dom32iopi.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/dom32iopi.cc -o $(OBJ)/dom32iopi.o
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
