clusters=$( expr $1 + 1 )
cd mpi_scripts/
python3 delegate_files.py $1
cd ..
mpirun -mca btl_tcp_if_exclude virbr0,docker0,lo -n $clusters --hostfile hostfile ./main.exe
# mpirun -n 1 ./main.exe : -n $1 --hostfile hostfile ./main.exe

# mpirun -n $1 --hostfile hostfile ./a.out
# mpirun -n 1 -host 127.0.0.1 ./a.out : -n $1 --hostfile hostfile ./a.out
# rm mpi_out/*.tar.gz
# mpirun -n 1 -host tablet ./a.out : -n  -host rapira ./a.out