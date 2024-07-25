<img width="407" alt="StacatoBanner" src="https://user-images.githubusercontent.com/42607186/159329357-e85d3a01-0044-4a98-aef5-efd307d0d704.png">

## Build Instructions (MacOS)

Execute the following commands, then generate project files with CMake.

```
git clone https://github.com/Guistac/Stacato.git --recursive
cd Stacato
git checkout 0.2
sh deps/openframeworks/scripts/osx/download_libs.sh
```


## Build Instructions (Linux)

Steps for a bare linux machine:
install vscode .deb file
instlal vscode extensions: cmaketools c/c++
run these commands:
```
sudo apt update
sudo apt updgrade
sudo apt install git cmake clang xorg-dev gtk+-3.0 wireshark
sudo apt-get install gtk+-3.0
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