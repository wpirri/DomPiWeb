#!/usr/bin/make -f

all:
	make -C dompi_abm
	make -C dompi_cloud
#	make -C dompi_garnet
	make -C dompi_gsm
	make -C dompi_netcomm
	make -C dompi_server
# Pi IO
	$(if $(findstring arm,$(MACHINE)) , make -C dompi_gpio )
	

clean:
	make -C dompi_abm clean
	make -C dompi_cloud clean
#	make -C dompi_garnet clean
	make -C dompi_gsm clean
	make -C dompi_netcomm clean
	make -C dompi_server clean
# Pi IO
	$(if $(findstring arm,$(MACHINE)) , make -C dompi_gpio clean )

install:
	make -C dompi_abm install
	make -C dompi_cloud install
#	make -C dompi_garnet install
	make -C dompi_gsm install
	make -C dompi_netcomm install
	make -C dompi_server install
# Pi IO
	$(if $(findstring arm,$(MACHINE)) , make -C dompi_gpio install )

uninstall:
	make -C dompi_abm uninstall
	make -C dompi_cloud uninstall
#	make -C dompi_garnet uninstall
	make -C dompi_gsm uninstall
	make -C dompi_netcomm uninstall
	make -C dompi_server uninstall
# Pi IO
	$(if $(findstring arm,$(MACHINE)) , make -C dompi_gpio uninstall )
