#!/usr/bin/make -f

all:
	make -C dompi_gpio
	make -C dompi_netcomm
	make -C dompi_server

clean:
	make -C dompi_gpio clean
	make -C dompi_netcomm clean
	make -C dompi_server clean

install:
	make -C dompi_gpio install
	make -C dompi_netcomm install
	make -C dompi_server install

uninstall:
	make -C dompi_gpio uninstall
	make -C dompi_netcomm uninstall
	make -C dompi_server uninstall

