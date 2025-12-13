#!/usr/bin/make -f

include ../configure.mk

install:
	$(CP) check_for_updates.sh /usr/local/sbin/
	$(CHMOD) 0755 /usr/local/sbin/check_for_updates.sh
	$(CP) make-dompidb-backup.sh /usr/local/sbin/
	$(CHMOD) 0755 /usr/local/sbin/make-dompidb-backup.sh
	$(CP) get_weather_data_open-meteo.sh /usr/local/sbin/
	$(CHMOD) 0755 /usr/local/sbin/get_weather_data_open-meteo.sh
	ln -f -s /usr/local/sbin/get_weather_data_open-meteo.sh /usr/local/sbin/get_weather_data.sh

	$(CP) make-db-backup /etc/cron.monthly/
	$(CHMOD) 0755 /etc/cron.monthly/make-db-backup
	$(CP) restart-60-seg /etc/cron.monthly/
	$(CHMOD) 0755 /etc/cron.monthly/restart-60-seg
	$(CP) update-weather /etc/cron.hourly/
	$(CHMOD) 0755 /etc/cron.hourly/update-weather

uninstall:
	$(RM) /usr/local/sbin/check_for_updates.sh
	$(RM) /usr/local/sbin/make-dompidb-backup.sh
	$(RM) /usr/local/sbin/get_weather_data_open-meteo.sh
	$(RM) /usr/local/sbin/get_weather_data.sh

	$(RM) /etc/cron.monthly/make-db-backup
	$(RM) /etc/cron.monthly/restart-60-seg
	$(RM) /etc/cron.hourly/update-weather
