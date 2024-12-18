#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>


void create_sbatch_script(int num_processes, const char *input_file) {
    FILE *script_file = fopen("tempjob.sh", "w");
    if (script_file == NULL) {
        perror("Cannot create sbatch script");
        exit(EXIT_FAILURE);
    }

    fprintf(script_file, "#! /bin/bash\n");
    fprintf(script_file, "#SBATCH --job-name=\"myHELLO\"\n");
    fprintf(script_file, "#SBATCH --partition=debug\n");
    fprintf(script_file, "#SBATCH --nodes=%d\n", num_processes);
    fprintf(script_file, "#SBATCH --time=0-00:05:00\n");
    fprintf(script_file, "#SBATCH --ntasks-per-node=1\n");
    fprintf(script_file, "#SBATCH --mem=1992\n");
    fprintf(script_file, "\n");
    
    // Используем snprintf для создания команды
    fprintf(script_file, "mpirun -np %d ./main_mpi  %s\n", num_processes, input_file);
    
    fclose(script_file);

    // Устанавливаем права на выполнение для скрипта
    system("chmod +x tempjob.sh");

    // Запускаем sbatch для запуска скрипта
    system("sbatch tempjob.sh");
}

void run_experiment(int *num_products_list, int num_products_count, int *num_processes_list, int num_processes_count) {
    FILE *result_file = fopen("experiment_results_openmpi.txt", "w");
    if (result_file == NULL) {
        perror("Cannot open results file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_products_count; i++) {
        int num_products = num_products_list[i];
        char input_file[30];
        snprintf(input_file, sizeof(input_file), "input_%d.txt", num_products);
        // generate_input_file(input_file, num_products, 10, 20); // Генерация файла

        // Запуск программы на 1 процессе для получения времени T1
        double start_time_1 = MPI_Wtime();
        char command[100];
//        snprintf(command, sizeof(command), "mpirun -np 1 ./main %s", input_file);
  //      system(command);
        double end_time_1 = MPI_Wtime();
        double T1;// = end_time_1 - start_time_1;

        for (int j = 0; j < num_processes_count; j++) {
            int num_processes = num_processes_list[j];
            double start_time = MPI_Wtime();
            
            // Создаем и запускаем sbatch скрипт для текущего числа процессов
            create_sbatch_script(num_processes, input_file);
            
            // Считываем время выполнения (поскольку скрипт запускается в асинхронном режиме, здесь можно добавить ожидание завершения if needed)
            double end_time = MPI_Wtime();
            double Tp = end_time - start_time;

            double S = (Tp > 0) ? T1 / Tp : 0;
            double E = (num_processes > 0) ? S / num_processes : 0;
        if (i == 0) {
             T1 = Tp;
        }
            fprintf(result_file, "%d %d %lf %lf %lf\n", num_products, num_processes, Tp, S, E);
        }
    }

    fclose(result_file);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    srand(time(NULL) + MPI_Wtime());

    int left = 200;
    int right = 200000;
    int num_products_list[30];
    int num_processes_list[] = {1, 2, 3, 4, 5, 6, 7, 8}; // Процессы для тестирования
    int num_products_count = 0;

    int step = (right - left) / 10;
    while (left <= right) {
        num_products_list[num_products_count++] = left;
        left += step;
    }

    run_experiment(num_products_list, num_products_count, num_processes_list, sizeof(num_processes_list) / sizeof(num_processes_list[0]));
    // plot_results();

    MPI_Finalize();
    return 0;
}
