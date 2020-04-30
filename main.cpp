#include "mpi.h"
// #include <stdio.h>
// #include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "image_handler.h"
#include "time.h"
#define MAX_LENGTH 250
#define TAG 0

int main(int argc, char *argv[])
{
    MPI_Init(NULL, NULL);
    int32_t rank;
    MPI_Status status;
    int32_t world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int32_t name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        char path[MAX_LENGTH];
        std::ifstream file;
        file.open("mpi_scripts/path_to_data.txt");
        file >> path;
        for (uint32_t i = 1; i < world_size; i++)
        {
            MPI_Send(path, MAX_LENGTH, MPI_CHAR, i, TAG, MPI_COMM_WORLD);
        }
    }
    else
    {
        system("mkdir -p mpi_tmp");
        char path[MAX_LENGTH];
        MPI_Recv(path, MAX_LENGTH, MPI_CHAR, 0, TAG, MPI_COMM_WORLD, &status);
        std::stringstream scpCommand, tarName, unTarCommand;
        std::string folder = "mpi_tmp/", rootName = "self", filenamesName = "filenames.txt";
        tarName << std::to_string(rank) << ".tar.gz";
        scpCommand << "scp " << rootName << ":" << path << tarName.str() << " " << folder;
        unTarCommand << "cd " << folder << " && "
                     << "tar -xf " << tarName.str();

        // std::cout << "______" << std::endl;

        // std::cout << processor_name << std::endl;
        // std::cout << scpCommand.str() << std::endl;
        // std::cout << unTarCommand.str() << std::endl;
        system(scpCommand.str().c_str());
        system(unTarCommand.str().c_str());

        std::ifstream filenames;
        std::string tmpFilename;
        filenames.open(folder + std::to_string(rank) + '/' + filenamesName);
        // std::cout << folder + std::to_string(rank) + '/' + filenamesName << std::endl;

        if (!filenames.is_open())
        {
            return 1;
        }
        int it = 0;

        Time handler;
        std::cout << "start CPU on " << processor_name << " with rank " << rank << std::endl;
        handler.start(ClockType::cpu);
        while (std::getline(filenames, tmpFilename))
        {
            // std::cout << tmpFilename << std::endl;
            tmpFilename = folder + std::to_string(rank) + '/' + tmpFilename;
            ImageHandler image(tmpFilename);
            image.grayConvert();
            image.save(tmpFilename);
        }
        handler.stop(ClockType::cpu);
        std::cout << "CPU time: " << handler.getElapsed(TimeType::milliseconds) << " miliseconds on " << processor_name << " with rank " << rank  << std::endl;
        std::stringstream tarCommand, scpToRootCommand;
        tarCommand << "cd " << folder << " && "<< "tar -zcf " << std::to_string(rank) << ".tar.gz " << std::to_string(rank) << "/";
        scpToRootCommand << "scp " << folder << std::to_string(rank) << ".tar.gz " << rootName << ":Documents/labs/cuda/5/mpi_out";
        system(tarCommand.str().c_str());
        system(scpToRootCommand.str().c_str());

        // std::cout << "______" << std::endl;
    }

    MPI_Finalize();
    return 0;
}
// tar - xf 2.tar.gz