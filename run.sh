if [ $1 ==  "make" ]
then
    # for HOST in self tablet rapira
    for HOST in self rapira
    do
        if [ $HOST == 'self' ]
        then
            mpic++ *.cpp -w -O3
        else
            scp *.cpp *.h $HOST:mpi_source
            # scp main.cpp $HOST:mpi_source
            ssh -t $HOST 'mpic++ mpi_source/*.cpp'
        fi
    done
else
    # mpirun -n $1 --hostfile hostfile ./a.out
    mpirun -n 1 -host 127.0.0.1 ./a.out : -n $1 --hostfile hostfile ./a.out
    rm mpi_out/*.tar.gz
    # mpirun -n 1 -host tablet ./a.out : -n  -host rapira ./a.out

fi

