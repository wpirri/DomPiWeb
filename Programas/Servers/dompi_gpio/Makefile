#!/usr/bin/make -f

include ../../../configure.mk

INCLUDE=-I../../Common/ -I/usr/local/include -I/usr/local/include/gmonitor
CXXFLAGS=$(PROCESSOR-PARAMS) -g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lwiringPi -lssl -lcrypto
OBJECTS=dompi_gpio.o dom32iopi.o config.o ctcp.o strfunc.o serial.o
OUTPUT=dompi_gpio
DEFINE=-D __NO__DEBUG__
HWNAME=uname -m
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_gpio

dompi_gpio.o: dompi_gpio.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_gpio.cc

config.o: ../../Common/config.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/config.cc

dom32iopi.o: ../../Common/dom32iopi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/dom32iopi.cc

ctcp.o: ../../Common/ctcp.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/ctcp.cc

strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc

serial.o: ../../Common/serial.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/serial.cc


dompi_gpio: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) -O $(OBJECTS) $(LIBS) -o $(OUTPUT)

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	$(CHMOD) +x install_server.sh
	./install_server.sh $(INST_SBINDIR) $(OUTPUT) $(INST_VARDIR)

uninstall:
	$(RM) $(INST_SBINDIR)/$(OUTPUT)
