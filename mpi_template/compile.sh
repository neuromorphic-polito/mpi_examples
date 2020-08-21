cd ./spinnnaker-software-stack/spinn_two
make
make install
cd ../../
cd ./spinnnaker-software-stack/spinn_acp
make
make install
cd ../../
cd ./spinnnaker-software-stack/spinn_mpi
make
make install
cd ../../
source step2_cmake_compile.sh
