git clone --depth=1 --recursive https://github.com/mgood7123/ManagedObject m
cd m
git clone --depth=1 --recursive https://github.com/Ravenbrook/mps mps_master
clear ; make CC=clang CXX=clang++ test_debug
