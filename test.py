import subprocess
import time
import numpy as np
import matplotlib.pyplot as plt

def generate_input_file(filename, num_products, num_vertices, num_edges):
    with open(filename, 'w') as f:
        f.write(f"{num_products}\n")
        f.write(f"{num_vertices}\n")
        f.write(f"{num_edges}\n")

        # Генерация вершин
        for _ in range(num_vertices):
            supplies = ' '.join(str(np.random.randint(-10, 10)) for _ in range(num_products))
            f.write(f"{supplies}\n")

        # Генерация ребер
        for _ in range(num_edges):
            from_vertex = np.random.randint(0, num_vertices)
            to_vertex = np.random.randint(0, num_vertices)
            weight = np.random.randint(1, 20)
            f.write(f"{from_vertex} {to_vertex} {weight}\n")

def run_experiment(num_products_list, num_processes_list):
    results = []

    # Запускаем эксперимент для всех комбинаций количества продуктов и процессов
    for num_products in num_products_list:
        input_file = f"input_{num_products}.txt"
        generate_input_file(input_file, num_products, 10, 20)  # Фиксируем количество вершин и ребер

        # Запускаем программу на 1 процессе, чтобы получить время T1
        start_time_1 = time.time()
        subprocess.run(["mpirun", "-np", "1", "./main", input_file])
        end_time_1 = time.time()
        T1 = end_time_1 - start_time_1

        for num_processes in num_processes_list:
            start_time = time.time()
            subprocess.run(["mpirun", "-np", str(num_processes), "./main", input_file])
            end_time = time.time()

            Tp = end_time - start_time
            S = T1 / Tp if Tp > 0 else 0
            E = S / num_processes if num_processes > 0 else 0

            results.append((num_products, num_processes, Tp, S, E))

    # Сохранение результатов в файл
    with open("experiment_results.txt", "w") as f:
        for num_products, num_processes, Tp, S, E in results:
            f.write(f"{num_products} {num_processes} {Tp} {S} {E}\n")

    return results

def plot_results(results):
    # Переводим результаты в формат numpy для удобной обработки
    num_products_list = sorted(list(set([r[0] for r in results])))
    num_processes_list = sorted(list(set([r[1] for r in results])))

    # Создаем матрицы для тепловых карт
    S_matrix = np.zeros((len(num_processes_list), len(num_products_list)))
    E_matrix = np.zeros((len(num_processes_list), len(num_products_list)))

    for i, num_processes in enumerate(num_processes_list):
        for j, num_products in enumerate(num_products_list):
            # Находим соответствующую запись в результатах
            for r in results:
                if r[0] == num_products and r[1] == num_processes:
                    S_matrix[i, j] = r[3]
                    E_matrix[i, j] = r[4]
                    break

    # Отрисовка тепловой карты для ускорения S
    plt.figure(figsize=(10, 6))
    plt.imshow(S_matrix, aspect='auto', origin='lower', cmap='viridis',
               extent=[min(num_products_list), max(num_products_list),
                       min(num_processes_list), max(num_processes_list)])
    plt.colorbar(label='Ускорение S')
    plt.xlabel('Количество продуктов')
    plt.ylabel('Количество процессов')
    plt.title('Тепловая карта ускорения S')
    plt.savefig('heatmap_S.png')
    plt.show()

    # Отрисовка тепловой карты для коэффициента полезного действия E
    plt.figure(figsize=(10, 6))
    plt.imshow(E_matrix, aspect='auto', origin='lower', cmap='viridis',
               extent=[min(num_products_list), max(num_products_list),
                       min(num_processes_list), max(num_processes_list)])
    plt.colorbar(label='Коэффициент полезного действия E')
    plt.xlabel('Количество продуктов')
    plt.ylabel('Количество процессов')
    plt.title('Тепловая карта коэффициента полезного действия E')
    plt.savefig('heatmap_E.png')
    plt.show()

if __name__ == "__main__":
    left = 0
    right = 2000
    generate_flag_product_list = True

    if generate_flag_product_list:
        num_products_list = []
        step = round((right - left) / 10)
        while left < right:
            left += step
            num_products_list.append(left)
    else:
        num_products_list = [10, 20, 30, 40, 50]

    num_processes_list = [1, 2, 3, 4, 5, 6, 7, 8]  # Изменил список процессов для более адекватных результатов

    results = run_experiment(num_products_list, num_processes_list)
    plot_results(results)