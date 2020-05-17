#include "mpi.h"
// #include <stdio.h>
// #include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

#include "image_handler.h"
#include "gaus_matrix.h"
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
    const int root = 0;

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int32_t name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::string whoAmI = processor_name;
    // std::cout << whoAmI << rank << std::endl;
    if (rank == root)
    {
        MPI_Request request = MPI_REQUEST_NULL;
        Time handler;
        handler.start(ClockType::cpu);
        char path[MAX_LENGTH];
        std::ifstream file;
        file.open("mpi_scripts/path_to_data.txt");
        file >> path;
        for (uint32_t i = 1; i < world_size; i++)
        {
            MPI_Send(path, MAX_LENGTH, MPI_CHAR, i, TAG, MPI_COMM_WORLD);
        }
        double elapsedTime = 0;
        for (size_t i = 1; i < world_size; i++)
        {
            double tmpElapsedTime = 0;
            char arr[sizeof(elapsedTime)];
            std::memcpy(arr,&elapsedTime,sizeof(elapsedTime));
            MPI_Recv(arr, sizeof(tmpElapsedTime), MPI_CHAR, i, TAG, MPI_COMM_WORLD, &status);
            std::memcpy(&tmpElapsedTime,arr,sizeof(tmpElapsedTime));
            std::cout << "Time - " << i << " " << tmpElapsedTime << std::endl;
            elapsedTime += tmpElapsedTime;
        }
        handler.stop(ClockType::cpu);
        std::cout << "Time - " << elapsedTime << std::endl;
        std::cout << "Time true - " << handler.getElapsed(TimeType::milliseconds) << std::endl;
    }
    else
    {
        char path[MAX_LENGTH];
        std::cout <<  whoAmI << std::endl;
        MPI_Recv(path, MAX_LENGTH, MPI_CHAR, root, TAG, MPI_COMM_WORLD, &status);
        std::stringstream scpCommand, tarName, unTarCommand;
        std::cout << path << " " << whoAmI << std::endl;
        std::string folder = "mpi_tmp/", rootName = "rapira", filenamesName = "filenames.txt";
        system("mkdir -p mpi_tmp");
        
        tarName << std::to_string(rank) << ".tar.gz";
        scpCommand << "scp " << rootName << ":" << path << tarName.str() << " " << folder;
        unTarCommand << "cd " << folder << " && " << "tar -xf " << tarName.str();
        system(scpCommand.str().c_str());
        system(unTarCommand.str().c_str());

        std::ifstream filenames;
        std::string tmpFilename;
        filenames.open(folder + std::to_string(rank) + '/' + filenamesName);
        // // std::cout << folder + std::to_string(rank) + '/' + filenamesName << std::endl;

        if (!filenames.is_open())
        {
            return 1;
        }
        int it = 0;

        Time handler;
        std::cout << "start GPU on " << processor_name << " with rank " << rank << std::endl;
        handler.start(ClockType::cpu);
        double elapsedTime = 0;
        while (std::getline(filenames, tmpFilename))
        {
            double tmpElapsedTime;
            // // std::cout << tmpFilename << std::endl;
            tmpFilename = folder + std::to_string(rank) + '/' + tmpFilename;
            ImageHandler image(tmpFilename);
            // image.grayConvert();
            image.gausFilterGpu(3,tmpElapsedTime);
            image.save(tmpFilename);
            elapsedTime += tmpElapsedTime;
        }
        handler.stop(ClockType::cpu);
        std::cout << "GPU time: " << handler.getElapsed(TimeType::milliseconds) << " miliseconds on " << processor_name << " with rank " << rank  << std::endl;

        std::stringstream tarCommand, scpToRootCommand, rmTmpFolderCommand, rmTrashFilesCommand;
        std::string destName = "artemr";

        std::cout << "back forward start " << processor_name << " with rank " << rank << std::endl;

        tarCommand << "cd " << folder << " && "<< "tar -zcf " << std::to_string(rank) << ".tar.gz " << std::to_string(rank) << "/";
        scpToRootCommand << "scp " << folder << std::to_string(rank) << ".tar.gz " << destName << ":Documents/labs/cuda/5/mpi_out";
        rmTmpFolderCommand << "rm -r " << folder;
        rmTrashFilesCommand << "rm " << folder << std::to_string(rank) << "/" << filenamesName;

        filenames.close();

        system(rmTrashFilesCommand.str().c_str());
        system(tarCommand.str().c_str());
        system(scpToRootCommand.str().c_str());
        system(rmTmpFolderCommand.str().c_str());

        char arr[sizeof(elapsedTime)];
        std::memcpy(arr,&elapsedTime,sizeof(elapsedTime));
        MPI_Send(arr, sizeof(elapsedTime), MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
// tar - xf 2.tar.gz