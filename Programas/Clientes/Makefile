#!/usr/bin/make -f

include ../../configure.mk

all:
	make -C abmalarma.cgi
	make -C abmassign.cgi
	make -C abmat.cgi
	make -C abmauto.cgi
	make -C abmev.cgi
	make -C abmgroup.cgi
	make -C abmhw.cgi
	make -C abmsys.cgi
	make -C abmuser.cgi
	make -C abmtouch.cgi
	make -C abmcamara.cgi
	make -C domcli
	make -C infoio.cgi
	make -C statusio.cgi
	make -C gmonitor_get_saf.cgi
	make -C dompi_mobile.cgi
# Pi IO
	$(if $(findstring arm,$(ARQ)) , make -C wifi.cgi )
	$(if $(findstring arm,$(ARQ)) , make -C iostatus.cgi )
	$(if $(findstring arm,$(ARQ)) , make -C ioswitch.cgi )
	$(if $(findstring arm,$(ARQ)) , make -C ioconfig.cgi )

clean:
	make -C abmalarma.cgi clean
	make -C abmassign.cgi clean
	make -C abmat.cgi clean
	make -C abmauto.cgi clean
	make -C abmev.cgi clean
	make -C abmgroup.cgi clean
	make -C abmhw.cgi clean
	make -C abmsys.cgi clean
	make -C abmtouch.cgi clean
	make -C abmuser.cgi clean
	make -C abmcamara.cgi clean
	make -C domcli clean
	make -C infoio.cgi clean
	make -C statusio.cgi clean
	make -C gmonitor_get_saf.cgi clean
	make -C dompi_mobile.cgi clean
# Pi IO
	$(if $(findstring arm,$(ARQ)) , make -C wifi.cgi clean )
	$(if $(findstring arm,$(ARQ)) , make -C iostatus.cgi clean )
	$(if $(findstring arm,$(ARQ)) , make -C ioswitch.cgi clean )
	$(if $(findstring arm,$(ARQ)) , make -C ioconfig.cgi clean )

install:
	make -C abmalarma.cgi install
	make -C abmassign.cgi install
	make -C abmat.cgi install
	make -C abmauto.cgi install
	make -C abmev.cgi install
	make -C abmgroup.cgi install
	make -C abmhw.cgi install
	make -C abmsys.cgi install
	make -C abmtouch.cgi install
	make -C abmuser.cgi install
	make -C abmcamara.cgi install
	make -C domcli install
	make -C infoio.cgi install
	make -C statusio.cgi install
	make -C gmonitor_get_saf.cgi install
	make -C dompi_mobile.cgi install
# Pi IO
	$(if $(findstring arm,$(ARQ)) , make -C wifi.cgi install )
	$(if $(findstring arm,$(ARQ)) , make -C iostatus.cgi install )
	$(if $(findstring arm,$(ARQ)) , make -C ioswitch.cgi install )
	$(if $(findstring arm,$(ARQ)) , make -C ioconfig.cgi install )

uninstall:
	make -C abmalarma.cgi uninstall
	make -C abmassign.cgi uninstall
	make -C abmat.cgi uninstall
	make -C abmauto.cgi uninstall
	make -C abmev.cgi uninstall
	make -C abmgroup.cgi uninstall
	make -C abmhw.cgi uninstall
	make -C abmsys.cgi uninstall
	make -C abmtouch.cgi uninstall
	make -C abmuser.cgi uninstall
	make -C abmcamara.cgi uninstall
	make -C domcli uninstall
	make -C infoio.cgi uninstall
	make -C statusio.cgi uninstall
	make -C gmonitor_get_saf.cgi uninstall
	make -C dompi_mobile.cgi uninstall
# Pi IO
	$(if $(findstring arm,$(ARQ)) , make -C wifi.cgi uninstall )
	$(if $(findstring arm,$(ARQ)) , make -C iostatus.cgi uninstall )
	$(if $(findstring arm,$(ARQ)) , make -C ioswitch.cgi uninstall )
	$(if $(findstring arm,$(ARQ)) , make -C ioconfig.cgi uninstall )
