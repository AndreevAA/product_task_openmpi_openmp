echo 'generated input '
gcc -o gen generate.c
./gen

echo '\nMPI started 200 - 20000, 20 blocks'
mpicc -o main main.c
mpicc -o manager manager.c -lm
./manager
echo '\nMPI ended!\n'

echo '\nMPI + OPENMP started 200 - 20000, 20 blocks'
mpicc -o main_openmp main_openmp.c
mpicc -o manager_openmp manager_openmp.c -lm
./manager_openmp
echo '\nMPI + OPENMP ended!\n'

# # sudo /usr/sbin/apachectl restart
# gcc -o main_threads main_threads.c -fopenmp
# gcc -o app_lpthreads manager_threads.c -fopenmp
# ./app_lpthreads 


rm venv
python3 -v venv venv
source venv/bin/activate
pip3 install numpy matplotlib sklearn
python3 plot_diagrams.py