#!/usr/bin/make -f
DOCUMENT_ROOT=/var/www/html/
CGI_ROOT=/usr/lib/cgi-bin/

CP=cp -va


#.cc.o:
#	$(CC) $(CFLAGS) $(CXXFLAGS) $(INCLUDE) $(DEFINE) -c $<

all: none

none:
	echo "implementado solamente make install"

install:
	$(CP) js $(DOCUMENT_ROOT)
	$(CP) css $(DOCUMENT_ROOT)
	$(CP) central $(DOCUMENT_ROOT)
	$(CP) images $(DOCUMENT_ROOT)
	$(CP) index.php $(DOCUMENT_ROOT)

