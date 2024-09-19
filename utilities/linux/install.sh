sudo mkdir /opt/Stacato
sudo cp ../../build/Stacato /opt/Stacato/Stacato
echo "installed executable"
sudo cp -r ../../dir/Resources /opt/Stacato/Resources
sudo cp ../../dir/Stacato_AppIcon.png /opt/Stacato/Stacato_AppIcon.png
sudo cp ../../dir/Stacato_FileIcon.png /opt/Stacato/Stacato_FileIcon.png
echo "installed resource files"

sudo cp Stacato.desktop /usr/share/applications/Stacato.desktop
echo "installed app shortcut"

sudo xdg-mime install --mode system stacato-projectfile.xml
sudo xdg-mime default Stacato.desktop stacato/projectfile

sudo xdg-icon-resource install --context mimetypes --mode system --size 1024 /opt/Stacato/Stacato_FileIcon.png stacato-projectfile

echo "installed stacato project file type"

sudo cp Stacato_Permissions /etc/sudoers.d/Stacato_Permissions
echo "installed launch permissions"

sudo cp Startup.sh /opt/Stacato/Startup.sh
sudo chmod +x /opt/Stacato/Startup.sh
echo "installed Autostart script to /opt/Stacato/Startup.h, add it to launch applications"
