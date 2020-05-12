cd ~/mpi_source
mv time.cpp time.cu
mv image_handler.cpp image_handler.cu
mv gaus_matrix.cpp gaus_matrix.cu
nvcc -c -arch=sm_35 -w *.cu
mpic++ -o main.exe *.cpp *.o -L/usr/lib/cuda/lib64 -lcudart 
mv main.exe ../