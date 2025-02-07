#get xinput device id for the weida touchscreen
#map it to the displayport-3 display so it doesn't cover other monitors
#set the touchscreen as primary display
#Launch Stacato

TouchDeviceID=$(xinput | grep -E 'Weida.*id=[0-9]+' | grep -Eo 'id=[0-9]+' | sed 's/id=//')
DisplayName=DisplayPort-3
xinput map-to-output $TouchDeviceID $DisplayName
xrandr --output $DisplayName --primary
sleep 3

sudo /opt/Stacato/Stacato