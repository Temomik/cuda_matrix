if [ $1 ==  "make" ]
then
    for HOST in rapira ptaxom
    # for HOST in self rapira
    do
        if [ $HOST == 'self' ]
        then
            mpic++ *.cpp -w -O3
        else
            ssh -t $HOST 'mkdir -p mpi_source && mkdir -p mpi_tmp'
            # scp *.cpp *.h $HOST:mpi_source
            scp main.cpp $HOST:mpi_source
            ssh -t $HOST 'mpic++ mpi_source/*.cpp'
        fi
    done
else
    # mpirun -n $1 --hostfile hostfile ./a.out
    cd mpi_scripts
    python3 delegate_files.py $1
    cd ..
    echo running
    mpirun -n 1 -host 127.0.0.1 ./a.out : -n $1 --hostfile hostfile ./a.out
    cd mpi_scripts
    # python3 untar.py $1
    # rm mpi_out/*.tar.gz
    # mpirun -n 1 -host tablet ./a.out : -n  -host rapira ./a.out

fi

# mpic++ *.o -lcudart -lcublas -L /usr/local/cuda/lib64 -I /usr/local/cuda/include
# nvcc -c *.cu
# mpic++ -c *.cpp