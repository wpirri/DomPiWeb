#!/usr/bin/make -f

INCLUDE=-I../../Common/
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lwiringPi
OBJECTS=dompi_netcomm.o qtcp.o dom32iowifi.o ctcp.o strfunc.o
OUTPUT=dompi_netcomm
DEFINE=-D N__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_netcomm

dompi_gpio.o: dompi_netcomm.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_netcomm.cc

ctcp.o: ../../Common/ctcp.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/ctcp.cc
qtcp.o: ../../Common/qtcp.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/qtcp.cc
strfunc.o: ../../Common/strfunc.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/strfunc.cc
dom32iowifi.o: ../../Common/dom32iowifi.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c ../../Common/dom32iowifi.cc

dompi_netcomm: $(OBJECTS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(LFLAGS) $(LIBS) -O $(OBJECTS) -o $(OUTPUT) 

clean:
	$(RM) $(OBJECTS) $(OUTPUT) *.d

install:
	cp $(OUTPUT) /usr/local/sbin/
	./update-tables.sh . /var/lib/gmonitor server /usr/local/sbin/$(OUTPUT)
	./update-tables.sh . /var/lib/gmonitor funcion

update:
	/usr/sbin/gmon_stop $(OUTPUT)
	sleep 1
	cp $(OUTPUT) /usr/local/sbin/
	/usr/sbin/gmon_start $(OUTPUT)

uninstall:
	$(RM) /usr/local/sbin/$(OUTPUT)