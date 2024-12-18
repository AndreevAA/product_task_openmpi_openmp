#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <limits.h>
#include <omp.h> // Включаем OpenMP

#define INF INT_MAX

typedef struct {
    int id;
    int *supply;   // Запасы или потребности по каждому продукту
} Vertex;

typedef struct {
    int from;
    int to;
    int weight;    // Стоимость перевозки по ребру
} Edge;

// Функция для чтения данных из файла
void readData(const char *filename, int *numProducts, int *numVertices, int *numEdges, 
              Vertex **vertices_ptr, Edge **edges_ptr) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Не удалось открыть файл %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    fscanf(file, "%d", numProducts);
    fscanf(file, "%d", numVertices);
    fscanf(file, "%d", numEdges);

    Vertex *vertices = (Vertex *)malloc((*numVertices) * sizeof(Vertex));
    Edge *edges = (Edge *)malloc((*numEdges) * sizeof(Edge));

    for (int i = 0; i < *numVertices; i++) {
        vertices[i].id = i;
        vertices[i].supply = (int *)malloc((*numProducts) * sizeof(int));
        for (int j = 0; j < *numProducts; j++) {
            fscanf(file, "%d", &vertices[i].supply[j]);
        }
    }

    for (int i = 0; i < *numEdges; i++) {
        fscanf(file, "%d %d %d", &edges[i].from, &edges[i].to, &edges[i].weight);
    }

    fclose(file);
    *vertices_ptr = vertices;
    *edges_ptr = edges;
}

// Освобождение памяти
void freeData(int numVertices, Vertex *vertices, Edge *edges) {
    for (int i = 0; i < numVertices; i++) {
        free(vertices[i].supply);
    }
    free(vertices);
    free(edges);
}

// Функция для применения алгоритма Беллмана-Форда
void bellmanFord(int start_vertex, int numVertices, Edge *edges, int numEdges, int *distances) {
    for (int i = 0; i < numVertices; i++) {
        distances[i] = INF;
    }
    distances[start_vertex] = 0;

    // Основной цикл алгоритма Беллмана-Форда (с использованием OpenMP)
    #pragma omp parallel for
    for (int i = 1; i < numVertices; i++) {
        // Создаем временные массивы для обновления расстояний
        int updated = 0; 
        for (int j = 0; j < numEdges; j++) {
            int u = edges[j].from;
            int v = edges[j].to;
            int weight = edges[j].weight;
            // Проверка и обновление расстояний
            #pragma omp critical
            {
                if (distances[u] != INF && distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    updated = 1; // Установим флаг обновления
                }
            }
        }
        if (!updated) break; // Если нет обновлений, выходим из цикла
    }
}

// Функция для решения подзадачи по одному продукту
void solveProductSubproblem(int productId, int numVertices, int numEdges, 
                            Vertex *vertices, Edge *edges, int rank, int size) {
    int distances[numVertices]; 

    for (int i = 0; i < numVertices; i++) {
        if (vertices[i].supply[productId] > 0) {
            // Запускаем алгоритм Беллмана-Форда для каждой вершины, у которой есть запасы данного продукта
            bellmanFord(i, numVertices, edges, numEdges, distances);

            // Подсчет транспортных потоков для данного продукта на основе расстояний
            for (int j = 0; j < numVertices; j++) {
                if (distances[j] < INF) {
                    // printf("Процесс %d: для продукта %d от вершины %d расстояние %d\n", rank, productId, i, distances[j]);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int numProducts = 0, numVertices = 0, numEdges = 0;
    Vertex *vertices = NULL;
    Edge *edges = NULL;

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        if (argc < 2) {
            // printf("Использование: %s <input_file>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        readData(argv[1], &numProducts, &numVertices, &numEdges, &vertices, &edges);
    }

    // Рассылаем размеры данных
    MPI_Bcast(&numProducts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&numEdges, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Индивидуальные процессы выделяют память для ребер
    edges = (Edge *)malloc(numEdges * sizeof(Edge));
    MPI_Bcast(edges, numEdges * sizeof(Edge), MPI_BYTE, 0, MPI_COMM_WORLD);

    // Каждый процесс выделяет память для вершин
    vertices = (Vertex *)malloc(numVertices * sizeof(Vertex));
    MPI_Bcast(vertices, numVertices * sizeof(Vertex), MPI_BYTE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < numVertices; i++) {
        vertices[i].supply = (int *)malloc(numProducts * sizeof(int));
        MPI_Bcast(vertices[i].supply, numProducts, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // Определение, какие продукты будет обрабатывать данный процесс
    int productsPerProcess = numProducts / size;
    int startProduct = rank * productsPerProcess;
    int endProduct = (rank == size - 1) ? numProducts : startProduct + productsPerProcess;

    // Обработка назначенных продуктов
    for (int p = startProduct; p < endProduct; p++) {
        solveProductSubproblem(p, numVertices, numEdges, vertices, edges, rank, size);
    }

    // Освобождение памяти
    freeData(numVertices, vertices, edges);

    MPI_Finalize();
    return 0;
}