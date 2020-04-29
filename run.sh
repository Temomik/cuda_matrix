if [ $1 ==  "make" ]
then
    for HOST in self tablet rapira
    do
        if [ $HOST == 'self' ]
        then
            mpic++ main.cpp
        else
            scp main.cpp $HOST:
            ssh -t $HOST 'mpic++ main.cpp'
        fi
    done
else
    # mpirun -n $1 --hostfile hostfile ./a.out
    mpirun -n 1 -host 127.0.0.1 ./a.out : -n $1 --hostfile hostfile ./a.out
    # mpirun -n 1 -host tablet ./a.out : -n  -host rapira ./a.out

fi

