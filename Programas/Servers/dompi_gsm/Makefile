#!/usr/bin/make -f

INCLUDE=-I../../Common/
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lgmc -lssl -lcrypto
OBJECTS=dompi_gsm.o config.o
OUTPUT=dompi_gsm
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_gsm

dompi_gsm.o: dompi_gsm.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_gsm.cc

config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc

dompi_gsm: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	chmod +x install_server.sh
	./install_server.sh $(OUTPUT)

uninstall:
	$(RM) /usr/local/sbin/$(OUTPUT)