#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// void generate_input_file(const char *filename, int num_products, int num_vertices, int num_edges) {
//     FILE *f = fopen(filename, "w");
//     if (f == NULL) {
//         perror("Cannot open file to write");
//         exit(EXIT_FAILURE);
//     }

//     fprintf(f, "%d\n%d\n%d\n", num_products, num_vertices, num_edges);

//     // Генерация вершин
//     for (int i = 0; i < num_vertices; i++) {
//         for (int j = 0; j < num_products; j++) {
//             fprintf(f, "%d ", rand() % 20 - 10); // Генерация от -10 до 9
//         }
//         fprintf(f, "\n");
//     }

//     // Генерация рёбер
//     for (int i = 0; i < num_edges; i++) {
//         int from_vertex = rand() % num_vertices;
//         int to_vertex = rand() % num_vertices;
//         int weight = rand() % 20 + 1; // Генерация веса от 1 до 20
//         fprintf(f, "%d %d %d\n", from_vertex, to_vertex, weight);
//     }

//     fclose(f);
// }

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
        snprintf(command, sizeof(command), "mpirun -np 1 ./main %s", input_file);
        system(command);
        double end_time_1 = MPI_Wtime();
        double T1 = end_time_1 - start_time_1;

        for (int j = 0; j < num_processes_count; j++) {
            int num_processes = num_processes_list[j];
            double start_time = MPI_Wtime();
            snprintf(command, sizeof(command), "mpirun -np %d ./main %s", num_processes, input_file);
            system(command);
            double end_time = MPI_Wtime();
            double Tp = end_time - start_time;

            double S = (Tp > 0) ? T1 / Tp : 0;
            double E = (num_processes > 0) ? S / num_processes : 0;

            fprintf(result_file, "%d %d %lf %lf %lf\n", num_products, num_processes, Tp, S, E);
        }
    }

    fclose(result_file);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    srand(time(NULL) + MPI_Wtime());

    int left = 200;
    int right = 20000;
    int num_products_list[30];
    int num_processes_list[] = {1, 2, 3, 4, 5, 6, 7, 8}; // Процессы для тестирования
    int num_products_count = 0;

    int step = (right - left) / 20;
    while (left <= right) {
        num_products_list[num_products_count++] = left;
        left += step;
    }

    run_experiment(num_products_list, num_products_count, num_processes_list, sizeof(num_processes_list) / sizeof(num_processes_list[0]));
    // plot_results();

    MPI_Finalize();
    return 0;
}