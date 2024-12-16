#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>

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

typedef struct {
    int productId;
    int numVertices;
    int numEdges;
    Vertex *vertices;
    Edge *edges;
    int threadId;
} ThreadData;

// Функция для чтения данных из файла
void readData(const char *filename, int *numProducts, int *numVertices, int *numEdges, 
              Vertex **vertices_ptr, Edge **edges_ptr) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Не удалось открыть файл %s\n", filename);
        exit(1);
    }

    fscanf(file, "%d", numProducts);
    fscanf(file, "%d", numVertices);
    fscanf(file, "%d", numEdges);

    // Выделение памяти для вершин и рёбер
    Vertex *vertices = (Vertex *)malloc((*numVertices) * sizeof(Vertex));
    Edge *edges = (Edge *)malloc((*numEdges) * sizeof(Edge));

    // Чтение вершин
    for (int i = 0; i < *numVertices; i++) {
        vertices[i].id = i;
        vertices[i].supply = (int *)malloc((*numProducts) * sizeof(int));
        for (int j = 0; j < *numProducts; j++) {
            fscanf(file, "%d", &vertices[i].supply[j]);
        }
    }

    // Чтение рёбер
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

// Функция для решения подзадачи по одному продукту
void *solveProductSubproblem(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    int productId = data->productId;
    int numVertices = data->numVertices;
    int numEdges = data->numEdges;
    Vertex *vertices = data->vertices;
    Edge *edges = data->edges;

    // Реализуем простой алгоритм для демонстрации вычислительной нагрузки
    int totalSupply = 0;
    for (int i = 0; i < numVertices; i++) {
        totalSupply += vertices[i].supply[productId];
    }

    int totalEdgeWeight = 0;
    for (int i = 0; i < numEdges; i++) {
        totalEdgeWeight += edges[i].weight;
    }

    for(int i = 0; i < 1000000; i++) {
        totalSupply = (totalSupply + totalEdgeWeight) % INF;
    }

    // Вывод результатов для данного продукта
    // printf("Поток %d: продукт %d, итоговый результат %d\n", data->threadId, productId, totalSupply);

    return NULL;
}

int main(int argc, char *argv[]) {
    int numProducts = 0, numVertices = 0, numEdges = 0, numThreads;
    Vertex *vertices = NULL;
    Edge *edges = NULL;

    if (argc < 3) {
        printf("Использование: %s <input_file>\n", argv[0]);
        printf("Количество потоков: %d\n", atoi(argv[1]));
        numThreads = atoi(argv[1]);
        exit(1);
    }
    
    readData(argv[1], &numProducts, &numVertices, &numEdges, &vertices, &edges);

    // int numThreads = 4; // Параметр для определения количества потоков
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];

    // Запускаем потоки для обработки продуктов
    for (int i = 0; i < numProducts; i++) {
        threadData[i % numThreads].productId = i;
        threadData[i % numThreads].numVertices = numVertices;
        threadData[i % numThreads].numEdges = numEdges;
        threadData[i % numThreads].vertices = vertices;
        threadData[i % numThreads].edges = edges;
        threadData[i % numThreads].threadId = i % numThreads;

        pthread_create(&threads[i % numThreads], NULL, solveProductSubproblem, &threadData[i % numThreads]);

        if (i % numThreads == numThreads - 1 || i == numProducts - 1) {
            for (int j = 0; j <= (i % numThreads); j++) {
                pthread_join(threads[j], NULL);
            }
        }
    }

    // Освобождение памяти
    freeData(numVertices, vertices, edges);

    return 0;
}