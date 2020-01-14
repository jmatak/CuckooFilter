# (Dynamic) Cuckoo Filter
Implementation of dynamic Cuckoo filter.
This project is developed within the course Bioinformatics at Faculty of Electrical Engineering and Computing,
University of Zagreb.
Link to the course pages: https://www.fer.unizg.hr/predmet/bio.

Follow the next example for compiling and running the demo example of (Dynamic) Cuckoo filter implementation. 
```
mkdir build
cd build
cmake ..
make
```

Run demo of plain Cuckoo filter:

```
./CuckooFilter
```

Run demo of dynamic Cuckoo filter:
```
./DynamicCuckooFilter
```


GitHub implementation: [CityHash](https://github.com/google/cityhash), fast and reliable hash function.

Installation of CityHash lib provided by their authors:
   > On systems with gcc, we generally recommend:

    ./configure
    make all check CXXFLAGS="-g -O3"
    sudo make install
    

###### Contributors:
   - Josip Jukić - Cuckoo Filter, Cuckoo Table, Dynamic Cuckoo Filter, Bit Manager
   - Patrik Marić - Cuckoo Filter, Hash Function, experiment scripts (Tests)
   - Josip Matak - Dynamic Cuckoo Filter, FASTA Reader, FASTA Iterator, Arg Parser
