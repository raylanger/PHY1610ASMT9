#include <mpi.h>
#include <random>
#include <rarray>

int main(){
    int rank, size;
    int N = 1'000'000'000, Z = 100'000;

    MPI_Init(nullptr,nullptr);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::uniform_real_distribution<double> uniform(0.0,1.0);
    std::mt19937 gen(std::random_device{}());

    
}