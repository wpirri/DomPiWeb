#!/usr/bin/make -f
DOCUMENT_ROOT=/var/www/html/
CGI_ROOT=/usr/lib/cgi-bin/

CP=cp -uva


#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: none

none:
	echo "implementado solamente make install"

install:
	mkdir -p $(DOCUMENT_ROOT)
	$(CP) * $(DOCUMENT_ROOT)
	mkdir -p $(DOCUMENT_ROOT)/upload
	chmod 0777 $(DOCUMENT_ROOT)/upload

uninstall:
	rm -rf $(DOCUMENT_ROOT)

