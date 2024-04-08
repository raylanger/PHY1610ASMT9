#!/bin/bash
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=16
#SBATCH --time=1:00:00
#SBATCH --mail-type=FAIL
#SBATCH --partition=debug

module load gcc/13 openmpi/5 rarray

#Define output file
OUTPUT_FILE1="main_output.txt"
OUTPUT_FILE2="main2_output.txt"
#Ensure output file blank
> $OUTPUT_FILE1
> $OUTPUT_FILE2

#echo -e "Threads\tTime" >> $OUTPUT_FILE

#Iterate through number of processors, timing main
for thread in 1 4 16 32
do
    #Specify number of processors used
    echo -e "Running with $thread Processors" >> $OUTPUT_FILE1

    # Time executbale with given number of processors
    { time mpirun -n $thread ./main ; } 2>> $OUTPUT_FILE1

    # Add newline
    echo -e "" >> $OUTPUT_FILE1
done

#Iterate through number of processors, timing main2
for thread in 1 4 16 32
do
    #Specify number of processors used
    echo -e "Running with $thread Processors" >> $OUTPUT_FILE2

    # Time executbale with given number of processors
    { time mpirun -n $thread ./main2 ; } 2>> $OUTPUT_FILE2

    # Add newline
    echo -e "" >> $OUTPUT_FILE2
done

                                                        
