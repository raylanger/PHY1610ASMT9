#include <mpi.h>
#include <random>
#include <rarray>
#include <iostream>

int main(){
    int rank, size;
    int N = 1'000'000'000, Z = 100'000;

    MPI_Init(nullptr,nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::uniform_real_distribution<double> uniform(0.0,1.0);
    std::mt19937 gen(std::random_device{}());
    rvector<int> total_counts(64);
    total_counts.fill(0);

    // std::cout << "Rank " << rank << " found " << counts << ".\n";
    rvector<int> send_counts(size);
    send_counts.fill(64/size);
    rvector<int> send_offsets(size);
    for (int i = 0; i<size;i++){
        send_offsets[i] = (64/size)*i;
    }

    rvector<int> send_rands(size);
    rvector<int> rands_offsets(size);
    send_rands.fill(Z/size);
    for (int i = 0; i < size; i++){
        rands_offsets[i] = i*Z/size;
    }

    rvector<int> counts(64/size);
    int index;
    double init = (rank+0.0)/size;
    double dx = 0.015625;
    
    for (int j = 0; j < N/Z; j ++){
        rvector<int> arr_sizes(size);
        rvector<int> offsets(size);
        rvector<double> rands(Z);
        rvector<double> rands_rank(Z/size);
        rvector<int> batch_counts(64);  
        if (rank == 0){
            if (j % 500 == 0){
                std::cout << "Batch "<<j <<"\n";
            }
        }

        gen.discard(Z*rank/size);
        for(int i = 0; i < Z/size; i++){
            rands_rank[i] = uniform(gen);
        }

        MPI_Gatherv(rands_rank.data(), Z/size, MPI_DOUBLE, rands.data(), send_rands.data(), rands_offsets.data(),MPI_DOUBLE, 0, MPI_COMM_WORLD);
        if (rank == 0){
            std::sort(rands.begin(),rands.end());
            // std::cout << rands << "\n";
            int counter = 0;
            double limit = (counter+1.0)/size;
            int data_init = 0;
            for (int i = 0; i < Z; i++){

                if(rands[i] > limit){
                    arr_sizes[counter] = i-data_init;
                    offsets[counter] = data_init;
                    counter += 1;
                    limit = (counter+1.0)/size;
                    data_init = i;
                }
            }
            arr_sizes[counter] = rands.size()-data_init;
            offsets[counter] = data_init;
        }

        MPI_Bcast(arr_sizes.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(offsets.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        rvector<double> recv_vals(arr_sizes[rank]);
        MPI_Scatterv(rands.data(), arr_sizes.data(), offsets.data(), MPI_DOUBLE, recv_vals.data(), arr_sizes[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        counts.fill(0);
        for (int i = 0; i < recv_vals.size(); i++){
            index = (recv_vals[i]-init)/dx;
            counts[index] += 1;
        }
        
        MPI_Gatherv(counts.data(), 64/size, MPI_INT, batch_counts.data(), send_counts.data(), send_offsets.data(),MPI_INT, 0, MPI_COMM_WORLD);
        if (rank == 0){
            // std::cout<<batch_counts<<"\n";
            for (int i = 0; i<64; i++){
                total_counts[i]+=batch_counts[i];
            }
        }
    }
    if (rank == 0){
        std::cout<<total_counts<<"\n";
    }
    MPI_Finalize();
    return 0;
}