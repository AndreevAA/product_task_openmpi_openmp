echo 'generated input '
gcc -o gen generate.c
./gen

echo '\nMPI started'
mpicc -o main_mpi main_mpi.c
mpicc -o manager_mpi manager_mpi.c -lm
./manager_mpi
echo '\nMPI ended!\n'

echo '\nMPI + OPENMP started'
mpicc -o main_mpi_openmp main_mpi_openmp.c
mpicc -o manager_mpi_openmp manager_mpi_openmp.c -lm
./manager_mpi_openmp
echo '\nMPI + OPENMP ended!\n'

rm venv
python3 -v venv venv
source venv/bin/activate
pip3 install numpy matplotlib sklearn
python3 plot_diagrams.py