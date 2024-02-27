#!/usr/bin/make -f

include ../configure.mk

	
install:
#	$(SQL) < create.sql
#	$(SQL) < init.sql
	$(CP) make_dump.sh  $(INST_BINDIR)/
	$(CHMOD) 0755 $(INST_BINDIR)/make_dump.sh
	$(CP) make_dump.sql $(INST_BINDIR)/
	$(CHMOD) 0644 $(INST_BINDIR)/make_dump.sql

uninstall:
	$(RM) $(INST_BINDIR)/make_dump.sh
	$(RM) $(INST_BINDIR)/make_dump.sql
