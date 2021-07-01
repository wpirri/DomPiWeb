#!/usr/bin/make -f

all:
	make -C dompi_gpio
	make -C dompi_netcom
	make -C dompi_server

clean:
	make -C dompi_gpio clean
	make -C dompi_netcom clean
	make -C dompi_server clean

install:
	make -C dompi_gpio install
	make -C dompi_netcom install
	make -C dompi_server install

uninstall:
	make -C dompi_gpio uninstall
	make -C dompi_netcom uninstall
	make -C dompi_server uninstall
