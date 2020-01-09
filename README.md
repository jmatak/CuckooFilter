# CuckooFilter
Implementation of cuckoo filter.

Follow the next example for compiling and running the code of Cuckoo filter implementation. 
```
mkdir build
cd build
cmake ..
make
./CuckooFilter -f PATH_TO_FASTA
```
  

GitHub implementation: [CityHash](https://github.com/google/cityhash), fast and reliable hash function.

Installation of CityHash lib provided by their authors:
   > On systems with gcc, we generally recommend:

    ./configure
    make all check CXXFLAGS="-g -O3"
    sudo make install