<img width="407" alt="StacatoBanner" src="https://user-images.githubusercontent.com/42607186/159329357-e85d3a01-0044-4a98-aef5-efd307d0d704.png">

## Build Instructions (MacOS)

Execute the following commands, then generate project files with CMake.

```
git clone https://github.com/Guistac/Stacato.git
cd Stacato
git checkout Linux2
git submodule init
git submodule update
```


## Build Instructions (Linux)

Steps for a bare linux machine:
- install vscode .deb file
- install vscode extensions: cmaketools c/c++
run these commands:
```
sudo apt update
sudo apt updgrade
sudo apt-get install git cmake clang xorg-dev gtk+-3.0 wireshark htop
```

To clone repo and start developping:
```
git clone https://github.com/Guistac/Stacato.git
cd Stacato
git checkout Linux2
git submodule init
git submodule update
code .
```



## Optimizing realtime performance on Linux

        ```
        Pirority stack:
        1 Hardware drivers
        2 Kernel
        3 SOEM + application
        4 All others
        ```

### Give priority to network drives or interrupts

- isolate some cpus by editing `/etc/default/grub`
- add `isolcpus=<cpu_list>` to `GRUB_CMDLINE_LINUX_DEFAULT` like this:
    `GRUB_CMDLINE_LINUX_DEFAULT="quiet splash isolcpus=2,3"`
- then do:
    ```
    sudo update-grub
    sudo reboot
    ```
- find the number for nic interrupts in file `/proc/interrupts`
- bind interrupts to cpus (using CPU affinity) like this
    ```
    echo [CpuMask] > /proc/irq/[irqNumber]/smp_affinity
    ```
    There might be other ways of setting this (`irqbalance` and `irqaffinity`)
    Apparrently with hyperthreading cpus only pairs of logical and physical cpus should be adressed

The main question is do we have to set this regularly or just once ?
We probably should do this only once on first configuration using a bash script
Or maybe out application can run bash commands to set cpu affinity for nic interrupts


### Give priority to stacato process or individual threads

All of this we do programmatically in our application code:

- give priority to a thread using the scheduling policies of `pthread` 
- set cpu affinity for realtime threads using `cpu_set_t` and `pthread_setaffinity_np`




## Register .Stacato Files on Linux Machines

```
cd /usr/share/applications
sudo nano Stacato.desktop
```

```
[Desktop Entry]
Name=Stacato
Exec=pkexec ~/Stacato/build/Stacato %F
Icon=~/Stacato/dir/Stacato_AppIcon.png
Type=Application
Terminal=true
Categories=CoolStuff
```

## Startup shortcut and root launch permissions

edit sudoers file to allow launching of the app without a password prompt:
`sudo visudo`
append the following to the end of the file and save:
`<username> ALL=(ALL) NOPASSWD: <path_to_executable>`
this gives the user permission to run this command as root without a password prompt


create the file ~/.local/share/applications/stacato.desktop file containing the following :

```
[Desktop Entry]
Version=1.0
Name=Stacato
Comment=Stage Control Automation Toolbox
Exec=sudo build/Stacato
Path=/home/<user>/Stacato/ %f
Icon=/home/<user>/Stacato/dir/Stacato_AppIcon.png
Type=Application
Terminal=true
MimeType=stacato/project
```

create the file stacato-project.xml
```
<?xml version="1.0"?>
<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>
    <mime-type type="stacato/project">
    <comment>Stacato Project File</comment>
    <glob pattern="*.stacato/"/>
    <glob pattern="*.Stacato/"/>
    </mime-type>
</mime-info>
```