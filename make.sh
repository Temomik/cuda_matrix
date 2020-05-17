cd ~/mpi_source
mv time.cpp time.cu
mv image_handler.cpp image_handler.cu
mv gaus_matrix.cpp gaus_matrix.cu
mv main.cpp main.cu
rm *.o
# nvcc -c -arch=sm_35 -dc -w *.cu
# nvcc -arch=sm_35 *.o -lcudadevrt
# mpic++ -c *.cppss
# mpic++ *.o -o main.exe -L/usr/lib/cuda/lib64 -lcudart 
# -X
nvcc --default-stream per-thread -w *.cu -o main.exe
# mpic++ -w -O3 -o main.exe *.cpp *.o -L/usr/lib/cuda/lib64 -lcudart 
rm *.cu
mv main.exe ../