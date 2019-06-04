if [ ! -d build ]; then
  mkdir  build
fi

cd build

cmake -DCUDA_SDK_ROOT_DIR=/usr/local/cuda/samples -DCMAKE_BUILD_TYPE=Release ../ 
-DCUDPP_ROOT_DIR=~/numerical/gvdb-voxels/build/shared_cudpp -DGVDB_ROOT_DIR=~/numerical/gvdb-voxels/install 
-DCMAKE_INSTALL_PREFIX=~/numerical/gvdb-voxels/install
