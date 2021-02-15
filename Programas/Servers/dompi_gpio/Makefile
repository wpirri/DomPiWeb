#!/usr/bin/make -f

INCLUDE=-I../../Common/
CXXFLAGS=-g -Wall -Wextra -MMD -O2
LIBS=-lgmqw -lgmshared -lstdc++ -lwiringPi
OBJECTS=dompi_gpio.o
OUTPUT=dompi_gpio
DEFINE=-D N__DEBUG__
CC=g++

#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: dompi_gpio

dompi_gpio.o: dompi_gpio.cc
	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c dompi_gpio.cc

dompi_gpio: $(OBJECTS)
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