sudo apt update
sudo apt install git cmake pkg-config build-essential manpages-dev gfortran wget

sudo mkdir -p /src
sudo chmod -R 777 /src

# Setup libsoduim
cd /src
wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.18-stable.tar.gz
tar -xzf libsodium-1.0.18-stable.tar.gz
cd /src/libsodium-stable
./configure
make -j4
make
make check
sudo make install
sudo ldconfig


# Setup MPI
cd /src
wget https://www.mpich.org/static/downloads/1.4/mpich2-1.4.tar.gz
tar -xzf mpich2-1.4.tar.gz
cd /src/mpich2-1.4
./configure
make -j4
make
sudo make install
sudo ldconfig