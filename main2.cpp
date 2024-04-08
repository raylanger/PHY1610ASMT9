#include <mpi.h>
#include <random>
#include <rarray>
#include <iostream>
#include "prng.h"

int main(){
    // Initialize rank and size variables
    int rank, size;

    // Set total size and batch size
    int N = 1'000'000'000, Z = 100'000;

    //Initialize MPI
    MPI_Init(nullptr,nullptr);

    // Get size of WORLD and rank of processor
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Initialize vector to hold total counts
    rvector<int> total_counts(64);
    total_counts.fill(0);

    // Create PRNG
    static PRNG gen(10);
    static std::uniform_real_distribution<> dis(0.0, 1.0);

    // Initiailize variables to control gathering of data
    rvector<int> send_counts(size);
    send_counts.fill(64/size);
    rvector<int> send_offsets(size);
    for (int i = 0; i<size;i++){
        send_offsets[i] = (64/size)*i;
    }

    // Initialize variables to control distribution of random numbers
    rvector<int> send_rands(size);
    rvector<int> rands_offsets(size);
    send_rands.fill(Z/size);
    for (int i = 0; i < size; i++){
        rands_offsets[i] = i*Z/size;
    }

    // Initialize variables to determine counts in this processor
    rvector<int> counts(64/size);
    int index;
    double init = (rank+0.0)/size;
    double dx = 0.015625;

    // Initialize vectors to contain the amount of data to send to eah processor and corresponding offsets in total vector
    rvector<int> arr_sizes(size);
    rvector<int> offsets(size);

    // Initiailize vector to hold random numbers for each batch
    rvector<double> rands(Z);

    // Initialize vector to hold random numbers for each rank
    rvector<double> rands_rank(Z/size);

    // Initialize vector to hold counts for this batch
    rvector<int> batch_counts(64);  

    // Discard values depending on rank such that each rank works on different random numbers
    int discard = (N*rank)/size;
    gen.discard(discard);

    for (int j = 0; j < N/Z; j ++){
        // Reset batch counts
         batch_counts.fill(0.0);

        // Progress counter
        if (rank == 0){
            if (j % 500 == 0){
                std::cout << "Batch "<<j <<"\n";
            }
        }
        
        // Generate random numbers on each processor  
        for(int i = 0; i < Z/size; i++){
            rands_rank[i] = dis(gen);
        }

        // Gather data onto root process
        MPI_Gather(rands_rank.data(), Z/size, MPI_DOUBLE, rands.data(), Z/size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        // Distribute data to each processor
        if (rank == 0){
            // Sort values
            std::sort(rands.begin(),rands.end());

            // Initialize variables to determine what data to send to each processor
            int counter = 0;
            double limit = (counter+1.0)/size;
            int data_init = 0;

            // Iterate through random numbers
            for (int i = 0; i < Z; i++){

                // Once upper limit of given rank exceeded, determine amount of data to send to that array, and where that data begins in the original array
                if(rands[i] > limit){
                    arr_sizes[counter] = i-data_init;
                    offsets[counter] = data_init;
                    counter += 1;
                    limit = (counter+1.0)/size;
                    data_init = i;
                }
            }
            //Set array size and offset of final rank
            arr_sizes[counter] = rands.size()-data_init;
            offsets[counter] = data_init;
        }

        // Broadcast array size and offsets to all processors
        MPI_Bcast(arr_sizes.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(offsets.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        
        // Create vector to hold received values
        rvector<double> recv_vals(arr_sizes[rank]);
        
        //Scatter data to all processors, dictating amount of data and its starting point for each rank using Scatterv
        MPI_Scatterv(rands.data(), arr_sizes.data(), offsets.data(), MPI_DOUBLE, recv_vals.data(), arr_sizes[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        // Set rank counts to 0
        counts.fill(0); 

        // Iterate through recieved values, determine counts
        for (int i = 0; i < recv_vals.size(); i++){
            index = (recv_vals[i]-init)/dx;
            counts[index] += 1;
        }

        //Gather counts from each processor to root process
        MPI_Gatherv(counts.data(), 64/size, MPI_INT, batch_counts.data(), send_counts.data(), send_offsets.data(),MPI_INT, 0, MPI_COMM_WORLD);
        
        //On root process, add to total counts
        if (rank == 0){
            for (int i = 0; i <64;i++){
                total_counts[i]+=batch_counts[i];
            }
        }
    }
    // Output total counts
    if (rank == 0){
        std::cout<<total_counts<<"\n";
    }
    MPI_Finalize();
    return 0;
}
