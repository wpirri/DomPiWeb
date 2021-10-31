#!/usr/bin/make -f

all:
	make -C dompi_gpio
	make -C dompi_netcomm
	make -C dompi_server
	make -C dompi_cloud

clean:
	make -C dompi_gpio clean
	make -C dompi_netcomm clean
	make -C dompi_server clean
	make -C dompi_cloud clean

install:
	make -C dompi_gpio install
	make -C dompi_netcomm install
	make -C dompi_server install
	make -C dompi_cloud install

uninstall:
	make -C dompi_gpio uninstall
	make -C dompi_netcomm uninstall
	make -C dompi_server uninstall
	make -C dompi_cloud uninstall

