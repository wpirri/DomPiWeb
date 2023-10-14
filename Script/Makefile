#!/usr/bin/make -f

include ../configure.mk

install:
	$(CP) clean-gmonitor-logs /etc/cron.daily/
	$(CHMOD) 0755 /etc/cron.daily/clean-gmonitor-logs
	$(CP) clean-gmonitor-old-logs.sh /usr/local/sbin/
	$(CHMOD) 0755 /usr/local/sbin/clean-gmonitor-old-logs.sh
	$(CP) dompi_check_for_updates.sh /usr/local/sbin/
	$(CHMOD) 0755 /usr/local/sbin/dompi_check_for_updates.sh
	$(CP) drop_ipc.sh /usr/local/sbin 
	$(CHMOD) 0755 /usr/local/sbin/drop_ipc.sh
#
	$(CP) gmonitor.service /lib/systemd/system/
	systemctl daemon-reload
	systemctl enable gmonitor.service
	systemctl start gmonitor.service
	$(CP) gmond /usr/local/sbin 
	$(CHMOD) 0755 /usr/local/sbin/gmond
	if test -e /etc/init.d/gmond ; then $(RM) /etc/init.d/gmond ; fi
	ln -s /usr/local/sbin/gmond /etc/init.d/gmond

uninstall:
	$(RM) /etc/cron.daily/clean-gmonitor-logs
	$(RM) /usr/local/sbin/clean-gmonitor-old-logs.sh
	$(RM) /usr/local/sbin/dompi_check_for_updates.sh
	$(RM) /usr/local/sbin/drop_ipc.sh

	systemctl stop gmonitor.service
	systemctl disable gmonitor.service
	$(RM) /lib/systemd/system/gmonitor.service
	systemctl daemon-reload
	$(RM) /usr/local/sbin/gmond
	$(RM) /etc/init.d/gmond
