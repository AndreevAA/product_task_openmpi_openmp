#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_input_file(const char *filename, int num_products, int num_vertices, int num_edges) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        perror("Cannot open file to write");
        exit(EXIT_FAILURE);
    }

    fprintf(f, "%d\n%d\n%d\n", num_products, num_vertices, num_edges);

    // Генерация вершин
    for (int i = 0; i < num_vertices; i++) {
        for (int j = 0; j < num_products; j++) {
            fprintf(f, "%d ", rand() % 20 - 10); // Генерация от -10 до 9
        }
        fprintf(f, "\n");
    }

    // Генерация рёбер
    for (int i = 0; i < num_edges; i++) {
        int from_vertex = rand() % num_vertices;
        int to_vertex = rand() % num_vertices;
        int weight = rand() % 20 + 1; // Генерация веса от 1 до 20
        fprintf(f, "%d %d %d\n", from_vertex, to_vertex, weight);
    }

    fclose(f);
}


int main() {
    srand(time(NULL));

    int left = 200;
    int right = 20000;
    int num_products_list[50];
    int num_processes_list[] = {1, 2, 3, 4, 5, 6, 7, 8}; // Процессы для тестирования
    int num_products_count = 0;

    int step = (right - left) / 20;
    while (left <= right) {
        num_products_list[num_products_count] = left;
	num_products_count += 1;
        left += step;
    }

    for (int i = 0; i < num_products_count; i++) {
            int num_products = num_products_list[i];
            char input_file[30];
            snprintf(input_file, sizeof(input_file), "input_%d.txt", num_products);
            generate_input_file(input_file, num_products, 10, 20); // Генерация файла

    }

    return 0;
}
