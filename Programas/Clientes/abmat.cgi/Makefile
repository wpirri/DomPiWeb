#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lstdc++ -lgmshared -lgmc -lcjson -lssl -lcrypto
OBJECTS=abmat.cgi.o config.o strfunc.o
OUTPUT=abmat.cgi
DEFINE=$(GENERAL_DEFINE)
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: abmat.cgi

abmat.cgi.o: abmat.cgi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c abmat.cgi.cc
config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) $(MODULOS_INCLUIDOS) -c ../../Common/strfunc.cc

abmat.cgi: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) $(LIBS) -O $(OBJECTS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	$(CP) $(OUTPUT) $(INST_CGIDIR)/

uninstall:
	$(RM) $(INST_CGIDIR)/$(OUTPUT)

