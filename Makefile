 #!/usr/bin/make -f

GMON_USER=gmonitor
DOMPI_INST_TEMP=/home/$(GMON_USER)
SYTEM_VAR=/var/$(GMON_USER)
UPDATE_FILE=gmonitor_dompiweb_update.tar.gz
INSTALL_FILE=gmonitor_dompiweb_install.tar.gz

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
	rm -rf $(DOMPI_INST_TEMP)
	mkdir -p $(DOMPI_INST_TEMP)
	mkdir -p $(DOMPI_INST_TEMP)/cgi
	mkdir -p $(DOMPI_INST_TEMP)/html

	cp -av Programas/Html/* $(DOMPI_INST_TEMP)/html/
	rm $(DOMPI_INST_TEMP)/html/Makefile
	cp Programas/Clientes/abmalarma.cgi/abmalarma.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmassign.cgi/abmassign.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmat.cgi/abmat.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmauto.cgi/abmauto.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmev.cgi/abmev.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmgroup.cgi/abmgroup.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmhw.cgi/abmhw.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmsys.cgi/abmsys.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/abmuser.cgi/abmuser.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/infoio.cgi/infoio.cgi $(DOMPI_INST_TEMP)/cgi/
	cp Programas/Clientes/statusio.cgi/statusio.cgi $(DOMPI_INST_TEMP)/cgi/
	
	tar cvzf $(UPDATE_FILE) --files-from=update-files.lst

	cp Database/create.sql $(DOMPI_INST_TEMP)/
	cp Database/init.sql $(DOMPI_INST_TEMP)/
	cp Script/gmond $(DOMPI_INST_TEMP)/
	cp Script/install.sh $(DOMPI_INST_TEMP)/
	cp Config/dompiweb.config $(DOMPI_INST_TEMP)/
	cp Config/funcion.tab $(DOMPI_INST_TEMP)/
	cp Config/funcion_parametro.tab $(DOMPI_INST_TEMP)/
	cp Config/server.tab $(DOMPI_INST_TEMP)/
	cp Config/server_parametro.tab $(DOMPI_INST_TEMP)/
	
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
	# * cd $(DOMPI_INST_TEMP)
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
