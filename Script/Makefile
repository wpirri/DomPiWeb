#!/usr/bin/make -f

include ../configure.mk

install:
	$(CP) check_for_updates.sh /usr/local/sbin/
	$(CHMOD) 0755 /usr/local/sbin/check_for_updates.sh
	$(CP) make-db-backup.sh /etc/cron.monthly/
	$(CHMOD) 0755 /etc/cron.monthly/make-db-backup.sh
	$(CP) restart-60-seg.sh /etc/cron.monthly/
	$(CHMOD) 0755 /etc/cron.monthly/restart-60-seg.sh
	$(CP) make-dompidb-backup.sh /usr/local/sbin/
	$(CHMOD) 0755 /usr/local/sbin/make-dompidb-backup.sh

uninstall:
	$(RM) /usr/local/sbin/check_for_updates.sh
	$(RM) /etc/cron.monthly/make-db-backup.sh
	$(RM) /usr/local/sbin/make-dompidb-backup.sh
