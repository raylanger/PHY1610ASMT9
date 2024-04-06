#include <mpi.h>
#include <random>
#include <rarray>
#include <iostream>

int main(){
    int rank, size;
    int N = 10, Z = 10;

    MPI_Init(nullptr,nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::uniform_real_distribution<double> uniform(0.0,1.0);
    std::mt19937 gen(std::random_device{}());

    for (int j = 0; j < N/Z; j ++){
        rvector<int> arr_sizes(size);
        rvector<int> offsets(size);
        rvector<double> rands(Z);

        if (rank == 0){
            for(int i = 0; i < Z; i++){
                rands[i] = uniform(gen);
            }
            
            std::sort(rands.begin(),rands.end());
            std::cout << rands << "\n";
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
            // std::cout << arr_sizes << "\n";
            // std::cout << offsets << "\n";
        }

        MPI_Bcast(arr_sizes.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(offsets.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
        rvector<double> recv_vals(arr_sizes[rank]);
        MPI_Scatterv(rands.data(), arr_sizes.data(), offsets.data(), MPI_DOUBLE, recv_vals.data(), arr_sizes[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
        std::cout << "Rank " << rank << " received " << recv_vals << ".\n";
        // std::cout << "Rank " << rank << " received " << arr_sizes[rank] << ", " << offsets[rank] << ".\n";
    }
    MPI_Finalize();
    return 0;
}