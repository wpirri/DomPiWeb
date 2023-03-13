 #!/usr/bin/make -f

GMON_USER=gmonitor
SYTEM_HOME=/home/$(GMON_USER)
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
	mkdir -p $(SYTEM_HOME)
	mkdir -p $(SYTEM_HOME)/cgi
	mkdir -p $(SYTEM_HOME)/html

	cp -av Programas/Html/* $(SYTEM_HOME)/html/
	rm $(SYTEM_HOME)/html/Makefile
	cp Programas/Clientes/abmassign.cgi/abmassign.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/abmat.cgi/abmat.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/abmauto.cgi/abmauto.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/abmev.cgi/abmev.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/abmgroup.cgi/abmgroup.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/abmhw.cgi/abmhw.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/abmsys.cgi/abmsys.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/abmuser.cgi/abmuser.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/infoio.cgi/infoio.cgi $(SYTEM_HOME)/cgi/
	cp Programas/Clientes/statusio.cgi/statusio.cgi $(SYTEM_HOME)/cgi/
	
	tar cvzf $(UPDATE_FILE) --files-from=update-files.lst

	cp Database/create.sql $(SYTEM_HOME)/
	cp Database/init.sql $(SYTEM_HOME)/
	cp Script/gmond $(SYTEM_HOME)/
	cp Script/install.sh $(SYTEM_HOME)/
	cp Config/dompiweb.config $(SYTEM_HOME)/
	cp Config/funcion.tab $(SYTEM_HOME)/
	cp Config/funcion_parametro.tab $(SYTEM_HOME)/
	cp Config/server.tab $(SYTEM_HOME)/
	cp Config/server_parametro.tab $(SYTEM_HOME)/
	
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
	# * cd $(SYTEM_HOME)
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
