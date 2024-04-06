#include <mpi.h>
#include <random>
#include <rarray>
#include <iostream>

int main(){
    int rank, size;
    int N = 100, Z = 10;

    MPI_Init(nullptr,nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::uniform_real_distribution<double> uniform(0.0,1.0);
    std::mt19937 gen(std::random_device{}());

    for (int j = 0; j < N/Z; j ++){
        rvector<int> arr_sizes(size-1);
        rvector<int> offsets(size-1);
        rvector<double> rands(Z);

        if (rank = 0){
            for(int i = 0; i < Z; i++){
                rands[i] = uniform(gen);
            }
            std::sort(rands.begin(),rands.end());

            int counter = 0;
            int data_init = 0;
            for (int i = 0; i < Z; i++){
                if(rands[i] > (counter+1)/size){
                    if (counter = 0){
                        int size_0 = i-1;
                        rvector<double> recv_vals(size_0);
                        for (int i = 0; i < size_0; i++){
                            recv_vals[i] = rands[i];
                        }
                        counter += 1;
                        data_init = i;
                    }else{
                        arr_sizes[counter-1] = i-data_init-1;
                        offsets[counter - 1] = data_init;
                        counter += 1;
                        data_init = i;
                    }
                }
            }
        }
        MPI_Bcast(arr_sizes.data(), size-1, MPI_INT, 0, MPI_COMM_WORLD);
        
        if(rank!=0){
            rvector<double> recv_vals(arr_sizes[rank-1]);
        }
        MPI_Bcast(offsets.data(), size-1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Scatterv(rands.data(), arr_sizes.data(), offsets.data(), MPI_DOUBLE, recv_vals.data(), arr_sizes.data(), 0, MPI_COMM_WORLD);
        std::cout << "Rank " << rank << " received " << recv_vals << ".\n" 
    }
    MPI_Finalize();
    return 0;
}