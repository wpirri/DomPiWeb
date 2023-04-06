#!/usr/bin/make -f

INCLUDE=-I../../Common/
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lgmc -lssl -lcrypto
OBJECTS=dompi_netcomm.o dom32iowifi.o ctcp.o strfunc.o config.o queue.o rbpiio.o
OUTPUT=dompi_netcomm
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_netcomm

dompi_netcomm.o: dompi_netcomm.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_netcomm.cc

queue.o: ../../Common/queue.c
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/queue.c
ctcp.o: ../../Common/ctcp.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/ctcp.cc
config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc
dom32iowifi.o: ../../Common/dom32iowifi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/dom32iowifi.cc
rbpiio.o: ../../Common/rbpiio.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/rbpiio.cc

dompi_netcomm: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	chmod +x install_server.sh
	./install_server.sh $(OUTPUT)

uninstall:
	$(RM) /usr/local/sbin/$(OUTPUT)
