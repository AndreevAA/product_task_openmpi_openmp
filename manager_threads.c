#include <stdio.h>
#include <stdlib.h>
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

void run_experiment(int *num_products_list, int num_products_count) {
    FILE *result_file = fopen("experiment_results_threads.txt", "w");
    if (result_file == NULL) {
        perror("Cannot open results file");
        exit(EXIT_FAILURE);
    }

    
    for (int i = 0; i < num_products_count; i++) {
        double T1;

        for (int num_threads = 1; num_threads <= 8; num_threads++) {

            int num_products = num_products_list[i];
            char input_file[30];
            snprintf(input_file, sizeof(input_file), "input_%d.txt", num_products);
            // generate_input_file(input_file, num_products, 10, 20); // Генерация файла

            // Пример запуска программы
            double start_time = (double)clock() / CLOCKS_PER_SEC;
            char command[100];
            snprintf(command, sizeof(command), "./main_threads %s %d", input_file, num_threads);
            system(command);
            double end_time = (double)clock() / CLOCKS_PER_SEC;
            double Tp = end_time - start_time;

            if (num_threads == 1) {
                T1 = Tp;
            }

            double S = (Tp > 0) ? T1 / Tp : 0;
            double E = (num_threads > 0) ? S / num_threads : 0;

            fprintf(result_file, "%d %d %lf %lf %lf\n", num_products, num_threads, Tp, S, E);
        }
    }

    fclose(result_file);
}

int main() {
    srand(time(NULL));

    int left = 200;
    int right = 2000;
    int num_products_list[10];
    int num_processes_list[] = {1, 2, 3, 4, 5, 6, 7, 8}; // Процессы для тестирования
    int num_products_count = 0;

    int step = (right - left) / 4;
    while (left <= right) {
        num_products_list[num_products_count++] = left;
        left += step;
    }
    
    run_experiment(num_products_list, num_products_count);

    return 0;
}