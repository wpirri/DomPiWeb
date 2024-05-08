#!/usr/bin/make -f

include ../../configure.mk

all: none

none:
	echo "implementado solamente make install"

install:
	$(MKDIR) $(INST_HTMLDIR)/
	$(CP_UVA) * $(INST_HTMLDIR)/
	$(MKDIR) $(INST_HTMLDIR)/upload
	$(CHMOD) 0777 $(INST_HTMLDIR)/upload
	$(MKDIR) $(INST_HTMLDIR)/download
	$(CHMOD) 0777 $(INST_HTMLDIR)/download

uninstall:
	$(RMR) $(INST_HTMLDIR)

