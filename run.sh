if [ $1 ==  "make" ]
then
    for HOST in ptaxom rapira
    do
        scp -q *.cpp *.h make.sh $HOST:mpi_source
        ssh -t $HOST 'sh mpi_source/make.sh'
    done
else
    scp -q start.sh hostfile rapira:
    scp -q mpi_scripts/delegate_files.py rapira:mpi_scripts/  
    ssh -t rapira  sh start.sh $1
    cd mpi_scripts
    python3 untar.py $1
    ssh -t rapira cd mpi_scripts && rm **tar**
    # mpirun -n $1 --hostfile hostfile ./a.out
    # mpirun -n 1 -host 127.0.0.1 ./a.out : -n $1 --hostfile hostfile ./a.out
    # rm mpi_out/*.tar.gz
    # mpirun -n 1 -host tablet ./a.out : -n  -host rapira ./a.out
fi