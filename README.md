<img width="407" alt="StacatoBanner" src="https://user-images.githubusercontent.com/42607186/159329357-e85d3a01-0044-4a98-aef5-efd307d0d704.png">

## Build Instructions (MacOS)

Execute the following commands, then generate project files with CMake.

```
git clone https://github.com/Guistac/Stacato.git
cd Stacato
git submodule init
git submodule update
```


## Build Instructions (Linux)


- install vscode (and extensions: cmaketools c/c++)
- run these commands:
```
sudo apt update
sudo apt updgrade
sudo apt-get install git cmake clang xorg-dev gtk+-3.0
git clone https://github.com/Guistac/Stacato.git
cd Stacato
git submodule init
git submodule update
code .
```
-set kit to Clang and build project



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

### Further Research topics for RT performance
- check what irqbalance does
- check the kernel boot parameter `rcu_nocbs=<CPU list>`

