#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
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

    // Выделение памяти для вершин и ребер
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

    // Чтение ребер
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

// Функция применения алгоритма Дейкстры для нахождения минимального пути
void dijkstra(int start_vertex, int numVertices, Edge *edges, int numEdges, int *distances) {
    int visited[numVertices];
    for (int i = 0; i < numVertices; i++) {
        distances[i] = INF;
        visited[i] = 0;
    }
    distances[start_vertex] = 0;

    for (int i = 0; i < numVertices - 1; i++) {
        // Находим вершину с минимальным расстоянием
        int min_distance = INF;
        int min_index = -1;
        for (int j = 0; j < numVertices; j++) {
            if (!visited[j] && distances[j] <= min_distance) {
                min_distance = distances[j];
                min_index = j;
            }
        }
        visited[min_index] = 1;

        // Обновляем расстояния соседних вершин
        for (int j = 0; j < numEdges; j++) {
            if (edges[j].from == min_index && !visited[edges[j].to] &&
                distances[min_index] != INF &&
                distances[min_index] + edges[j].weight < distances[edges[j].to]) {
                distances[edges[j].to] = distances[min_index] + edges[j].weight;
            }
        }
    }
}

// Функция для решения подзадачи по одному продукту
void solveProductSubproblem(int productId, int numVertices, int numEdges, 
                            Vertex *vertices, Edge *edges, int rank) {
    // Создание массива для расстояний
    int distances[numVertices];
    for (int i = 0; i < numVertices; i++) {
        dijkstra(i, numVertices, edges, numEdges, distances);

        // Подсчет транспортных потоков для данного продукта на основе расстояний
        for (int j = 0; j < numVertices; j++) {
            // Учитываем только те вершины, которые имеют дефицит или избыток данного продукта
            if (vertices[i].supply[productId] > 0) {
                // printf("Процесс %d: для продукта %d от вершины %d расстояние %d\n", rank, productId, j, distances[j]);
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

    MPI_Datatype MPI_Edge_Type;
    MPI_Datatype MPI_Vertex_Type;

    // Создаем MPI тип для Edge
    int edge_block_lengths[3] = {1, 1, 1};
    MPI_Aint edge_displacements[3];
    MPI_Aint edge_addresses[3], edge_start_address;
    Edge edge_example;

    MPI_Get_address(&edge_example, &edge_start_address);
    MPI_Get_address(&edge_example.from, &edge_addresses[0]);
    MPI_Get_address(&edge_example.to, &edge_addresses[1]);
    MPI_Get_address(&edge_example.weight, &edge_addresses[2]);

    edge_displacements[0] = edge_addresses[0] - edge_start_address;
    edge_displacements[1] = edge_addresses[1] - edge_start_address;
    edge_displacements[2] = edge_addresses[2] - edge_start_address;

    // FILE *outputFile = NULL;
    // if (rank == 0) {
    //     outputFile = fopen("output_mpi.txt", "w");
    //     if (!outputFile) {
    //         perror("Не удалось открыть файл для записи результатов работы");
    //         MPI_Abort(MPI_COMM_WORLD, 1);
    //     }
    // }


    MPI_Datatype edge_types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Type_create_struct(3, edge_block_lengths, edge_displacements, edge_types, &MPI_Edge_Type);
    MPI_Type_commit(&MPI_Edge_Type);

    // Чтение и рассылка данных
    if (rank == 0) {
        if (argc < 2) {
            printf("Использование: %s <input_file>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        readData(argv[1], &numProducts, &numVertices, &numEdges, &vertices, &edges);

        // Так как размер supply у каждой вершины может быть разным, нужно передать данные корректно
        // Отправим размеры данных всем процессам
        MPI_Bcast(&numProducts, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numEdges, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Передаем ребра
        MPI_Bcast(edges, numEdges, MPI_Edge_Type, 0, MPI_COMM_WORLD);

        // Передаем вершины
        // Сначала передаем идентификаторы вершин
        int *vertex_ids = (int *)malloc(numVertices * sizeof(int));
        for (int i = 0; i < numVertices; i++) {
            vertex_ids[i] = vertices[i].id;
        }
        MPI_Bcast(vertex_ids, numVertices, MPI_INT, 0, MPI_COMM_WORLD);
        free(vertex_ids);

        // Затем передаем матрицу supply
        int *supply_matrix = (int *)malloc(numVertices * numProducts * sizeof(int));
        for (int i = 0; i < numVertices; i++) {
            for (int j = 0; j < numProducts; j++) {
                supply_matrix[i * numProducts + j] = vertices[i].supply[j];
            }
        }
        MPI_Bcast(supply_matrix, numVertices * numProducts, MPI_INT, 0, MPI_COMM_WORLD);
        free(supply_matrix);
    } else {
        // Получаем размеры данных
        MPI_Bcast(&numProducts, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numEdges, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Выделяем память для ребер и получаем их
        edges = (Edge *)malloc(numEdges * sizeof(Edge));
        MPI_Bcast(edges, numEdges, MPI_Edge_Type, 0, MPI_COMM_WORLD);

        // Получаем идентификаторы вершин
        vertices = (Vertex *)malloc(numVertices * sizeof(Vertex));
        int *vertex_ids = (int *)malloc(numVertices * sizeof(int));
        MPI_Bcast(vertex_ids, numVertices, MPI_INT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < numVertices; i++) {
            vertices[i].id = vertex_ids[i];
        }
        free(vertex_ids);

        // Получаем матрицу supply
        int *supply_matrix = (int *)malloc(numVertices * numProducts * sizeof(int));
        MPI_Bcast(supply_matrix, numVertices * numProducts, MPI_INT, 0, MPI_COMM_WORLD);
        for (int i = 0; i < numVertices; i++) {
            vertices[i].supply = (int *)malloc(numProducts * sizeof(int));
            for (int j = 0; j < numProducts; j++) {
                vertices[i].supply[j] = supply_matrix[i * numProducts + j];
            }
        }
        free(supply_matrix);
    }

    MPI_Type_free(&MPI_Edge_Type);

    // Определение, какие продукты будет обрабатывать данный процесс
    int productsPerProcess = numProducts / size;
    int extra = numProducts % size;
    int startProduct, endProduct;

    if (rank < extra) {
        startProduct = rank * (productsPerProcess + 1);
        endProduct = startProduct + productsPerProcess;
    } else {
        startProduct = rank * productsPerProcess + extra;
        endProduct = startProduct + productsPerProcess - 1;
    }

    // Обработка назначенных продуктов
    for (int p = startProduct; p <= endProduct && p < numProducts; p++) {
        solveProductSubproblem(p, numVertices, numEdges, vertices, edges, rank);
    }

    // Освобождение памяти
    freeData(numVertices, vertices, edges);

    MPI_Finalize();
    return 0;
}