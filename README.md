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
Priority stack:
1 Hardware drivers
2 Kernel
3 RT Thread of application
4 All others
```

- Enable realtime kernel using `ubuntu pro`
- Edit the kernel boot parameters to isolate and reduce jitter in cpus 2 and 3 
    ```
    sudo nano /etc/default/grub
    ```
- add `nohz=on nohz_full=2,3 isolcpus=2,3 irqaffinity=0-1,4-15` to `GRUB_CMDLINE_LINUX`
- then do:
    ```
    sudo update-grub
    sudo reboot
    ```
- Disable dynamic cpu frequency, sleep states and anything useless in the machine BIOS configuration
- Execute the script `utilities/linux/NicIrqAffinity.sh` at login to bind the IRQ for our fieldbus NIC to core 2

### Further Research topics for RT performance
- check what irqbalance does
- check the kernel boot parameter `rcu_nocbs=<CPU list>`

