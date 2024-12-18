#! /bin/bash
#SBATCH --job-name="myHELLO"
#SBATCH --partition=debug
#SBATCH --nodes=8
#SBATCH --time=0-00:05:00
#SBATCH --ntasks-per-node=1
#SBATCH --mem=1992

mpirun -np 8 ./main_mpi  input_200000.txt
