 #!/usr/bin/make -f

include configure.mk

all:
	make -C Programas

clean:
	make -C Programas clean

install:
	make -C Script install
	make -C Programas install

uninstall:
	make -C Script uninstall
	make -C Programas uninstall

installer:
	rm -rf $(RUN_HOME)
	mkdir -p $(RUN_HOME)
	mkdir -p $(RUN_HOME)/cgi
	mkdir -p $(RUN_HOME)/html

	cp -av Programas/Html/* $(RUN_HOME)/html/
	rm $(RUN_HOME)/html/Makefile
	cp Programas/Clientes/abmalarma.cgi/abmalarma.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmassign.cgi/abmassign.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmat.cgi/abmat.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmauto.cgi/abmauto.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmcamara.cgi/abmcamara.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmev.cgi/abmev.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmgroup.cgi/abmgroup.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmhw.cgi/abmhw.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmsys.cgi/abmsys.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmuser.cgi/abmuser.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/infoio.cgi/infoio.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/statusio.cgi/statusio.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/gmonitor_get_saf.cgi/gmonitor_get_saf.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_mobile.cgi/dompi_mobile.cgi $(RUN_HOME)/cgi/
	
	tar cvzf $(UPDATE_FILE) --files-from=update-files.lst

	cp Database/create.sql $(RUN_HOME)/
	cp Database/init.sql $(RUN_HOME)/
	cp Script/gmond $(RUN_HOME)/
	cp Script/install.sh $(RUN_HOME)/
	cp Config/dompiweb.config $(RUN_HOME)/
	cp Config/funcion.tab $(RUN_HOME)/
	cp Config/funcion_parametro.tab $(RUN_HOME)/
	cp Config/server.tab $(RUN_HOME)/
	cp Config/server_parametro.tab $(RUN_HOME)/
	
	tar cvzf $(INSTALL_FILE) --files-from=install-files.lst

	#
	# *******************************************************************************"
	# * Para instalar el sistema:
	# * 
	# * copiar gmonitor_dompiweb_install.tar.gz a /home/gmonitor
	# * ejecutar con el usuario root:
	# * 
	# * cd /
	# * tar xvzf /home/gmonitor/gmonitor_dompiweb_install.tar.gz
	# * cd $(RUN_HOME)
	# * ./install.sh
	# * 
	# *******************************************************************************"
	# * Para actualizar el sistema:
	# * 
	# * Copiar gmonitor_dompiweb_update.tar.gz a /home/gmonitor
	# * Reiniciar el sistema (/etc/init.d/gmond restart)
	# * 
	# *******************************************************************************"
	#
