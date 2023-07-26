#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lstdc++ -lsqlite3 -lcjson
OBJECTS=dbutil.o csqlite.o
OUTPUT=dbutil
DEFINE=-D __DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dbutil

dbutil.o: dbutil.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dbutil.cc

csqlite.o: ../../Common/csqlite.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/csqlite.cc

dbutil: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) $(LIBS) -O $(OBJECTS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	echo "Nada para hacer"

uninstall:
	echo "Nada para hacer"
