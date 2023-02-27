#!/usr/bin/make -f

INCLUDE=-I../../Common/
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lstdc++ -lgmshared -lgmc -lcjson -lssl -lcrypto
OBJECTS=abmgroup.cgi.o config.o strfunc.o
OUTPUT=abmgroup.cgi
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: abmgroup.cgi

abmgroup.cgi.o: abmgroup.cgi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c abmgroup.cgi.cc
config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc

abmgroup.cgi: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) $(LIBS) -O $(OBJECTS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	cp $(OUTPUT) /usr/lib/cgi-bin/

uninstall:
	$(RM) /usr/lib/cgi-bin/$(OUTPUT)

