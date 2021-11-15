#!/usr/bin/make -f

INCLUDE=-I../../Common/
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lcjson -lwiringPi
OBJECTS=dompi_gpio.o dom32iopi.o
OUTPUT=dompi_gpio
DEFINE=-D __NO__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_gpio

dompi_gpio.o: dompi_gpio.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_gpio.cc

dompi_gpio: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) $(LIBS) -O $(OBJECTS) -o $(OUTPUT) 

dom32iopi.o: ../../Common/dom32iopi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/dom32iopi.cc

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	./install_server.sh $(OUTPUT)

uninstall:
	$(RM) /usr/local/sbin/$(OUTPUT)