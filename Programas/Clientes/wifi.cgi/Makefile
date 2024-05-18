#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lstdc++ -lgmshared -lgmc -lcjson -lssl -lcrypto
DEFINE=$(GENERAL_DEFINE)
CC=g++
OBJECTS=$(OBJ)/wifi.cgi.o $(OBJ)/config.o $(OBJ)/strfunc.o
OUTPUT=$(PROG)/$(EXE)
EXE=wifi.cgi

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: $(OUTPUT)

$(OBJ)/wifi.cgi.o: wifi.cgi.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c wifi.cgi.cc -o $(OBJ)/wifi.cgi.o
$(OBJ)/config.o: ../../Common/config.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/config.cc -o $(OBJ)/config.o
$(OBJ)/strfunc.o: ../../Common/strfunc.cc
	$(MKDIR) $(OBJ)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/strfunc.cc -o $(OBJ)/strfunc.o

$(OUTPUT): $(OBJECTS)
	$(MKDIR) $(PROG)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RMR) $(MACHINE)

install:
	$(CP) $(OUTPUT) $(INST_CGIDIR)/

uninstall:
	$(RM) $(INST_CGIDIR)/$(EXE)
