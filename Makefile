 #!/usr/bin/make -f

# =============================================================================
# instalar
#	Para Desarrollo:  xinetd apache2 php libapache2-mod-php default-libmysqlclient-dev libcjson-dev libssl-dev [default-mysql-server]
#	Para Home Server: xinetd dapache2 php libapache2-mod-php default-mysql-server libcjson1 libssl
#

include configure.mk

all:
	make -C Programas

clean:
	make -C Programas clean

install:
	make -C Database install
	make -C Script install
	make -C Programas install

uninstall:
	make -C Database uninstall
	make -C Script uninstall
	make -C Programas uninstall

installer:
	rm -rf $(RUN_HOME)
	mkdir -p $(RUN_HOME)
	mkdir -p $(RUN_HOME)/cgi
	mkdir -p $(RUN_HOME)/html

	cp -av Programas/Html/* $(RUN_HOME)/html/
	rm $(RUN_HOME)/html/Makefile
	cp Programas/Clientes/abmalarma.cgi/$(PROG)/abmalarma.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmassign.cgi/$(PROG)/abmassign.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmat.cgi/$(PROG)/abmat.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmauto.cgi/$(PROG)/abmauto.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmcamara.cgi/$(PROG)/abmcamara.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmev.cgi/$(PROG)/abmev.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmgroup.cgi/$(PROG)/abmgroup.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmhw.cgi/$(PROG)/abmhw.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmsys.cgi/$(PROG)/abmsys.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/abmuser.cgi/$(PROG)/abmuser.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/infoio.cgi/$(PROG)/infoio.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/statusio.cgi/$(PROG)/statusio.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/gmonitor_get_saf.cgi/$(PROG)/gmonitor_get_saf.cgi $(RUN_HOME)/cgi/
	cp Programas/Clientes/dompi_mobile.cgi/$(PROG)/dompi_mobile.cgi $(RUN_HOME)/cgi/

	$(if $(findstring arm,$(MACHINE)) , cp Programas/Clientes/ioconfig.cgi/$(PROG)/ioconfig.cgi $(RUN_HOME)/cgi/ )
	$(if $(findstring arm,$(MACHINE)) , cp Programas/Clientes/iostatus.cgi/$(PROG)/iostatus.cgi $(RUN_HOME)/cgi/ )
	$(if $(findstring arm,$(MACHINE)) , cp Programas/Clientes/ioswitch.cgi/$(PROG)/ioswitch.cgi $(RUN_HOME)/cgi/ )
	$(if $(findstring arm,$(MACHINE)) , cp Programas/Clientes/wifi.cgi/$(PROG)/wifi.cgi $(RUN_HOME)/cgi/ )

	$(if $(findstring arm,$(MACHINE)) , tar cvzf $(UPDATE_FILE_ARM) --files-from=update-files-arm.lst , tar cvzf $(UPDATE_FILE_I386) --files-from=update-files-i386.lst )

	cp Database/create.sql $(RUN_HOME)/
	cp Database/init.sql $(RUN_HOME)/
	cp Script/install.sh $(RUN_HOME)/
	cp Config/dompiweb.config $(RUN_HOME)/
	cp Config/funcion.tab $(RUN_HOME)/
	cp Config/funcion_parametro.tab $(RUN_HOME)/
	cp Config/server.tab $(RUN_HOME)/
	cp Config/server_parametro.tab $(RUN_HOME)/
	
	$(if $(findstring arm,$(MACHINE)) , tar cvzf $(INSTALL_FILE_ARM) --files-from=install-files-arm.lst , tar cvzf $(INSTALL_FILE_I386) --files-from=install-files-i386.lst )

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

test:
	@echo "RUN_USER:      "$(RUN_USER)
	@echo "RUN_HOME:      "$(RUN_HOME)
	@echo "INST_USER:     "$(INST_USER)
	@echo "INST_LIBDIR:   "$(INST_LIBDIR)
	@echo "INST_HEADDIR:  "$(INST_HEADDIR)
	@echo "INST_BINDIR:   "$(INST_BINDIR)
	@echo "INST_SBINDIR:  "$(INST_SBINDIR)
	@echo "INST_ETCDIR:   "$(INST_ETCDIR)
	@echo "INST_VARDIR:   "$(INST_VARDIR)
	@echo "INST_INCDIR:   "$(INST_INCDIR)
	@echo "INST_LOGDIR:   "$(INST_LOGDIR)
	@echo "INST_CGIDIR:   "$(INST_CGIDIR)
	@echo "INST_HTMLDIR:  "$(INST_HTMLDIR)
	@echo "DATABASE:      "$(DATABASE)
	@echo "SQL:           "$(SQL)
	@echo "UPDATE_FILE:   "$(UPDATE_FILE)
	@echo "INSTALL_FILE:  "$(INSTALL_FILE)
	@echo "MACHINE:       "$(MACHINE)
	@echo "OBJ:           "$(OBJ)
	@echo "PROG:          "$(PROG)
	@echo "INST:          "$(INST)

	$(if $(findstring arm,$(MACHINE)) ,@echo "Compilando para RaspBerry Pi", @echo "Compilando para PC" )

fixeol:
	dos2unix fixeol.sh
	./fixeol.sh

