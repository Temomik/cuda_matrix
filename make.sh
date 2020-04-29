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