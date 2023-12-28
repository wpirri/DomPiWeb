#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lstdc++ -lgmshared -lgmc -lcjson -lssl -lcrypto
OBJECTS=ioswitch.cgi.o config.o strfunc.o
OUTPUT=ioswitch.cgi
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: ioswitch.cgi

ioswitch.cgi.o: ioswitch.cgi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ioswitch.cgi.cc
config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc

ioswitch.cgi: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	cp $(OUTPUT) /usr/lib/cgi-bin/

uninstall:
	$(RM) /usr/lib/cgi-bin/$(OUTPUT)

