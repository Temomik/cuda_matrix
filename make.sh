echo building
cd ~/Documents/cudaProj/5
rm main.exe
mv main.cpp main.cu
mv time.cpp time.cu
mv image_handler.cpp image_handler.cu
mv gaus_matrix.cpp gaus_matrix.cu
nvcc *.cu -o main.exe -w -O3
echo running
./main.exe  

# nvcc -c -O3  -arch sm_30  -I/home/rapira/opt/usr/local/include -I/home/rapira/opt/usr/local/include/openmpi *.cu -w
# nvcc -lm  -lcudart -lcublas  -L/home/rapira/opt/usr/local/lib/  -lmpi -lopen-rte -lopen-pal -ldl -lnsl -lutil -lm *.o -o a.out






mpiicc *.o -lcudart -lcublas -L usr/local/cuda/lib64 -I usr/local/cuda/include