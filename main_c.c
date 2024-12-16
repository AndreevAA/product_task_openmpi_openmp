#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <limits.h>

#define INF INT_MAX  // Определяем бесконечность как максимальное значение для типа int

// Структура для представления вершины графа (например, поставки или потребности)
typedef struct {
    int id;          // Уникальный идентификатор вершины
    int *supply;    // Запасы или потребности по каждому продукту
} Vertex;

// Структура для представления ребра графа (например, путь между двумя местами)
typedef struct {
    int from;       // Начальная вершина
    int to;         // Конечная вершина
    int weight;     // Стоимость перевозки по ребру
} Edge;

// Функция для чтения данных из файла
void readData(const char *filename, int *numProducts, int *numVertices, int *numEdges, 
              Vertex **vertices_ptr, Edge **edges_ptr) {
    FILE *file = fopen(filename, "r");  // Открываем файл для чтения
    if (!file) {
        // Если файл не удалось открыть, выводим сообщение об ошибке и завершаем программу
        printf("Не удалось открыть файл %s\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    // Считываем количество продуктов, вершин и ребер из файла
    fscanf(file, "%d", numProducts);
    fscanf(file, "%d", numVertices);
    fscanf(file, "%d", numEdges);

    // Выделяем память для массивов вершин и ребер
    Vertex *vertices = (Vertex *)malloc((*numVertices) * sizeof(Vertex));
    Edge *edges = (Edge *)malloc((*numEdges) * sizeof(Edge));

    // Читаем данные о вершинах
    for (int i = 0; i < *numVertices; i++) {
        vertices[i].id = i;  // Устанавливаем уникальный идентификатор вершины
        vertices[i].supply = (int *)malloc((*numProducts) * sizeof(int));  // Выделяем память для запасов
        for (int j = 0; j < *numProducts; j++) {
            fscanf(file, "%d", &vertices[i].supply[j]);  // Считываем запасы для каждого продукта
        }
    }

    // Читаем данные о ребрах
    for (int i = 0; i < *numEdges; i++) {
        fscanf(file, "%d %d %d", &edges[i].from, &edges[i].to, &edges[i].weight);  // Считываем информацию о ребре
    }

    fclose(file);  // Закрываем файл

    // Передаем указатели на массивы вершин и ребер
    *vertices_ptr = vertices;
    *edges_ptr = edges;
}

// Функция для освобождения выделенной памяти
void freeData(int numVertices, Vertex *vertices, Edge *edges) {
    // Освобождаем память для запасов каждой вершины
    for (int i = 0; i < numVertices; i++) {
        free(vertices[i].supply);
    }
    free(vertices);  // Освобождаем память для массива вершин
    free(edges);     // Освобождаем память для массива ребер
}

// Функция для решения подзадачи по одному продукту
void solveProductSubproblem(int productId, int numVertices, int numEdges, 
                            Vertex *vertices, Edge *edges, int rank) {
    // Пример простого алгоритма для обработки одного продукта

    int totalSupply = 0;  // Переменная для хранения общего предложения
    // Подсчитываем общее количество предложений по данному продукту
    for (int i = 0; i < numVertices; i++) {
        totalSupply += vertices[i].supply[productId];
    }

    int totalEdgeWeight = 0;  // Переменная для хранения общего веса ребер
    // Подсчитываем общий вес всех ребер
    for (int i = 0; i < numEdges; i++) {
        totalEdgeWeight += edges[i].weight;
    }

    // Вывод результатов для данного продукта можно комментировать или выводить по необходимости
    // printf("Процесс %d: продукт %d, итоговый результат %d\n", rank, productId, totalSupply);
}

int main(int argc, char *argv[]) {
    int numProducts = 0, numVertices = 0, numEdges = 0;  // Переменные для хранения количества продуктов, вершин и ребер
    Vertex *vertices = NULL;  // Указатель на массив вершин
    Edge *edges = NULL;       // Указатель на массив ребер

    MPI_Init(&argc, &argv);  // Инициализация MPI

    int rank, size;          // Переменные для хранения номера процесса и общего количества процессов
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Получаем номер текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Получаем общее количество процессов

    MPI_Datatype MPI_Edge_Type;  // Объявляем пользовательский тип MPI для ребра
    MPI_Datatype MPI_Vertex_Type; // Объявляем пользовательский тип MPI для вершины (не используется в данной версии кода)

    // Создаем MPI тип для Edge
    int edge_block_lengths[3] = {1, 1, 1};  // Объявляем длины блоков в структуре ребра
    MPI_Aint edge_displacements[3];          // Указываем смещения для каждого поля
    MPI_Aint edge_addresses[3], edge_start_address;
    Edge edge_example;  // Пример ребра для определения адресов

    // Получаем адреса полей структуры edge_example
    MPI_Get_address(&edge_example, &edge_start_address);
    MPI_Get_address(&edge_example.from, &edge_addresses[0]);
    MPI_Get_address(&edge_example.to, &edge_addresses[1]);
    MPI_Get_address(&edge_example.weight, &edge_addresses[2]);

    // Рассчитываем смещения для каждого поля
    edge_displacements[0] = edge_addresses[0] - edge_start_address;
    edge_displacements[1] = edge_addresses[1] - edge_start_address;
    edge_displacements[2] = edge_addresses[2] - edge_start_address;

    MPI_Datatype edge_types[3] = {MPI_INT, MPI_INT, MPI_INT};  // Указываем типы полей
    MPI_Type_create_struct(3, edge_block_lengths, edge_displacements, edge_types, &MPI_Edge_Type);  // Создаем пользовательский тип MPI
    MPI_Type_commit(&MPI_Edge_Type);  // Подтверждаем создание типа

    // Чтение и рассылка данных
    if (rank == 0) {  // Код только для процесса с номером 0 (мастер)
        if (argc < 2) {
            printf("Использование: %s <input_file>\n", argv[0]);  // Проверка наличия аргумента командной строки
            MPI_Abort(MPI_COMM_WORLD, 1);  // Если файл не указан, завершаем работу
        }
        // Чтение данных из файла
        readData(argv[1], &numProducts, &numVertices, &numEdges, &vertices, &edges);

        // Рассылаем размеры данных всем процессам
        MPI_Bcast(&numProducts, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numEdges, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Передаем ребра всем процессам
        MPI_Bcast(edges, numEdges, MPI_Edge_Type, 0, MPI_COMM_WORLD);

        // Передаем идентификаторы вершин всем процессам
        int *vertex_ids = (int *)malloc(numVertices * sizeof(int));
        for (int i = 0; i < numVertices; i++) {
            vertex_ids[i] = vertices[i].id;  // Сохраняем идентификаторы в массив
        }
        MPI_Bcast(vertex_ids, numVertices, MPI_INT, 0, MPI_COMM_WORLD);  // Рассылаем идентификаторы
        free(vertex_ids);  // Освобождаем память

        // Затем передаем матрицу supply всем процессам
        int *supply_matrix = (int *)malloc(numVertices * numProducts * sizeof(int));
        for (int i = 0; i < numVertices; i++) {
            for (int j = 0; j < numProducts; j++) {
                supply_matrix[i * numProducts + j] = vertices[i].supply[j];  // Сохраняем данные по запасам в одномерный массив
            }
        }
        MPI_Bcast(supply_matrix, numVertices * numProducts, MPI_INT, 0, MPI_COMM_WORLD);  // Рассылаем данные по запасам
        free(supply_matrix);  // Освобождаем память
    } else {
        // Получаем размеры данных для других процессов
        MPI_Bcast(&numProducts, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numVertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&numEdges, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Выделяем память для ребер и получаем их
        edges = (Edge *)malloc(numEdges * sizeof(Edge));
        MPI_Bcast(edges, numEdges, MPI_Edge_Type, 0, MPI_COMM_WORLD);  // Рассылаем структуру ребер

        // Получаем массив вершин
        vertices = (Vertex *)malloc(numVertices * sizeof(Vertex));
        int *vertex_ids = (int *)malloc(numVertices * sizeof(int));
        MPI_Bcast(vertex_ids, numVertices, MPI_INT, 0, MPI_COMM_WORLD);  // Рассылаем идентификаторы вершин
        for (int i = 0; i < numVertices; i++) {
            vertices[i].id = vertex_ids[i];  // Устанавливаем идентификатор для каждой вершины
        }
        free(vertex_ids);  // Освобождаем память

        // Получаем матрицу supply
        int *supply_matrix = (int *)malloc(numVertices * numProducts * sizeof(int));
        MPI_Bcast(supply_matrix, numVertices * numProducts, MPI_INT, 0, MPI_COMM_WORLD);  // Рассылаем данные по запасам
        for (int i = 0; i < numVertices; i++) {
            vertices[i].supply = (int *)malloc(numProducts * sizeof(int));  // Выделяем память для запасов каждой вершины
            for (int j = 0; j < numProducts; j++) {
                vertices[i].supply[j] = supply_matrix[i * numProducts + j];  // Заполняем массив запасов
            }
        }
        free(supply_matrix);  // Освобождаем память
    }

    MPI_Type_free(&MPI_Edge_Type);  // Освобождаем пользовательский тип MPI

    // Определение, какие продукты будет обрабатывать данный процесс
    int productsPerProcess = numProducts / size;  // Вычисляем количество продуктов на процесс
    int extra = numProducts % size;                // Находим остаток от деления (дополнительные продукты для некоторых процессов)
    int startProduct, endProduct;                  // Переменные для хранения диапазона продуктов для обработки процессом

    // Распределяем продукты между процессами
    if (rank < extra) {
        startProduct = rank * (productsPerProcess + 1);  // Начало диапазона для процесса с остатком
        endProduct = startProduct + productsPerProcess;    // Конец диапазона
    } else {
        startProduct = rank * productsPerProcess + extra;   // Начало диапазона для процесса без остатка
        endProduct = startProduct + productsPerProcess - 1; // Конец диапазона
    }

    // Обработка назначенных продуктов
    for (int p = startProduct; p <= endProduct && p < numProducts; p++) {
        solveProductSubproblem(p, numVertices, numEdges, vertices, edges, rank);  // Вызываем функцию для решения подзадачи
    }

    // Освобождение памяти
    freeData(numVertices, vertices, edges);  // Освобождаем память для данных

    MPI_Finalize();  // Завершение работы MPI
    return 0;  // Завершаем программу
}
