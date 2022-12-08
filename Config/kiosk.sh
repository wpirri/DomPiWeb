#!/bin/bash
xset s noblank
xset s off
xset -dpms

unclutter -idle 0.5 -root &

sed -i 's/"exited_cleanly":false/"exited_cleanly":true/' /home/pi/.config/chromium/Default/Preferences
sed -i 's/"exit_type":"Crashed"/"exit_type":"Normal"/' /home/pi/.config/chromium/Default/Preferences

rm -rf /home/pi/.cache/?hromium/?efault/?ache/*

if [ -x /usr/bin/chromium-browser ]; then
        /usr/bin/chromium-browser --noerrdialogs --disable-infobars --kiosk http://192.168.10.24/
else
        /usr/bin/chromium --noerrdialogs --disable-infobars --kiosk http://192.168.10.24/
fi
