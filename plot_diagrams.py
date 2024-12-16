import subprocess
import time
import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression

def read_all_information(openmpi_file="experiment_results_openmpi.txt", 
                         threads_file="experiment_results_openmp.txt"):
    results_openmpi, result_openmp = [], []

    tmp_results_openmpi_file = open(openmpi_file, "r").read()
    tmp_results_openmpi_file_line = tmp_results_openmpi_file.split("\n")

    tmp_result_openmp_file = open(threads_file, "r").read()
    tmp_result_openmp_file_line = tmp_result_openmp_file.split("\n")

    if len(tmp_results_openmpi_file_line) != len(tmp_result_openmp_file_line):
        print("ОШИБКА КОЛИЧЕСТВА СТРОК В ФАЙЛАХ")
        print(len(tmp_results_openmpi_file_line), len(tmp_result_openmp_file_line))
        print(tmp_results_openmpi_file_line, tmp_result_openmp_file_line)
        exit()

    for _ in range(len(tmp_results_openmpi_file_line) - 1):
        tmp_line_openmpi = tmp_results_openmpi_file_line[_]
        tmp_line_threads = tmp_result_openmp_file_line[_]

        tmp_line_openmpi_elems = tmp_line_openmpi.split()
        tmp_line_threads_elems = tmp_line_threads.split()

        # Используем float для преобразования типов
        results_openmpi.append([int(tmp_line_openmpi_elems[0]), 
                                int(tmp_line_openmpi_elems[1]), 
                                float(tmp_line_openmpi_elems[2]), 
                                float(tmp_line_openmpi_elems[3]), 
                                float(tmp_line_openmpi_elems[4])])
        
        result_openmp.append([int(tmp_line_threads_elems[0]), 
                                int(tmp_line_threads_elems[1]), 
                                float(tmp_line_threads_elems[2]), 
                                float(tmp_line_threads_elems[3]), 
                                float(tmp_line_threads_elems[4])])

    return results_openmpi, result_openmp

def plot_performance_metrics(results_openmpi, result_openmp):
    """Визуализация коэффициента полезного действия и ускорения для OpenMPI и OpenMP."""
    products = sorted(set(r[0] for r in results_openmpi))
    num_processes = sorted(set(r[1] for r in results_openmpi))

    # Словари для хранения значений E и S
    efficiency_openmpi = np.zeros((len(products), len(num_processes)))
    acceleration_openmpi = np.zeros((len(products), len(num_processes)))
    
    # Рассчитываем эффективность и ускорение для OpenMPI
    for i, product in enumerate(products):
        one_thread_time = None
        for r in result_openmp:
            if r[0] == product and r[1] == 1:  # только один поток
                one_thread_time = r[2]
                break
        
        for j, num_proc in enumerate(num_processes):
            omp_time = None
            for r in results_openmpi:
                if r[0] == product and r[1] == num_proc:
                    omp_time = r[2]
                    break
            
            if omp_time and one_thread_time:
                efficiency_openmpi[i, j] = one_thread_time / omp_time
                acceleration_openmpi[i, j] = one_thread_time / omp_time

    # Построение тепловых карт для OpenMPI
    plt.figure(figsize=(12, 6))
    plt.subplot(1, 2, 1)
    plt.title("Коэффициент полезного действия (E) OpenMPI")
    plt.xlabel("Количество процессов")
    plt.ylabel("Размер продукта")
    plt.imshow(efficiency_openmpi, aspect='auto', cmap='viridis', 
               extent=[num_processes[0], num_processes[-1], products[-1], products[0]], 
               origin='upper')
    plt.colorbar(label="E (КПД)")

    plt.subplot(1, 2, 2)
    plt.title("Ускорение (S) OpenMPI")
    plt.xlabel("Количество процессов")
    plt.ylabel("Размер продукта")
    plt.imshow(acceleration_openmpi, aspect='auto', cmap='plasma', 
               extent=[num_processes[0], num_processes[-1], products[-1], products[0]], 
               origin='upper')
    plt.colorbar(label="S (Ускорение)")

    plt.tight_layout()
    plt.savefig('performance_metrics_heatmap_OPENMPI.png')
    # plt.show()

    # Аналогичные расчёты для OpenMP
    products_threads = sorted(set(r[0] for r in result_openmp))
    num_processes_threads = sorted(set(r[1] for r in result_openmp))
    
    efficiency_openmp = np.zeros((len(products_threads), len(num_processes_threads)))
    acceleration_openmp = np.zeros((len(products_threads), len(num_processes_threads)))

    for i, product in enumerate(products_threads):
        one_thread_time = None
        for r in result_openmp:
            if r[0] == product and r[1] == 1:  # только один поток
                one_thread_time = r[2]
                break
        
        for j, num_proc in enumerate(num_processes_threads):
            omp_time = None
            for r in result_openmp:
                if r[0] == product and r[1] == num_proc:
                    omp_time = r[2]
                    break
            
            if omp_time and one_thread_time:
                efficiency_openmp[i, j] = one_thread_time / omp_time
                acceleration_openmp[i, j] = one_thread_time / omp_time

    # Построение тепловых карт для OpenMP
    plt.figure(figsize=(12, 6))
    plt.subplot(1, 2, 1)
    plt.title("Коэффициент полезного действия (E) OpenMP")
    plt.xlabel("Количество потоков")
    plt.ylabel("Размер продукта")
    plt.imshow(efficiency_openmp, aspect='auto', cmap='viridis', 
               extent=[num_processes_threads[0], num_processes_threads[-1], products_threads[-1], products_threads[0]], 
               origin='upper')
    plt.colorbar(label="E (КПД)")

    plt.subplot(1, 2, 2)
    plt.title("Ускорение (S) OpenMP")
    plt.xlabel("Количество потоков")
    plt.ylabel("Размер продукта")
    plt.imshow(acceleration_openmp, aspect='auto', cmap='plasma', 
               extent=[num_processes_threads[0], num_processes_threads[-1], products_threads[-1], products_threads[0]], 
               origin='upper')
    plt.colorbar(label="S (Ускорение)")

    plt.tight_layout()
    plt.savefig('performance_metrics_heatmap_OPENMP.png')
    # plt.show()
    plt.close()

def plot_results(results_openmpi, result_openmp):
    # Группируем результаты по размеру продукта для построения графиков
    products = sorted(set(r[0] for r in results_openmpi))  # размеры продуктов
    num_processes = sorted(set(r[1] for r in results_openmpi))  # количества процессов

    for product in products:
        # Подготавливаем данные для текущего размера продукта
        openmpi_times = []
        threads_times = []

        for num_proc in num_processes:
            # Ищем соответствующее время выполнения в результатах openmpi
            omp_time = None
            for r in results_openmpi:
                if r[0] == product and r[1] == num_proc:
                    omp_time = r[2]  # Время выполнения openmpi
                    break
            openmpi_times.append(omp_time)

            # Ищем соответствующее время выполнения в результатах threads
            thread_time = None
            for r in result_openmp:
                if r[0] == product and r[1] == num_proc:
                    thread_time = r[2]  # Время выполнения threads
                    break
            threads_times.append(thread_time)

        # Построение графика
        plt.figure(figsize=(10, 6))
        plt.plot(num_processes, openmpi_times, marker='o', label='OpenMPI')
        plt.plot(num_processes, threads_times, marker='s', label='OpenMPI + OpenMP')
        plt.xlabel('Количество процессов')
        plt.ylabel('Время выполнения (время в секунду)')
        plt.title(f'Время выполнения для размера продукта {product}')
        plt.legend()
        plt.grid()
        plt.xticks(num_processes)  # Установить метки по оси X
        plt.savefig(f'time_execution_product_{product}.png')
        # plt.show()
        plt.close()

def plot_results_all_in_one(results_openmpi, result_openmp):
    

    # Группируем результаты по размеру продукта для построения графиков
    products = sorted(set(r[0] for r in results_openmpi))  # размеры продуктов
    num_processes = sorted(set(r[1] for r in results_openmpi))  # количества процессов

    # Создаем один общий график для всех продуктов
    plt.figure(figsize=(12, 8))

    for product in products:
        # Подготавливаем данные для текущего размера продукта
        openmpi_times = []
        threads_times = []

        for num_proc in num_processes:
            # Ищем соответствующее время выполнения в результатах openmpi
            for r in results_openmpi:
                if r[0] == product and r[1] == num_proc:
                    openmpi_times.append(r[2])  # Время выполнения openmpi
                    break
            
            # Ищем соответствующее время выполнения в результатах threads
            for r in result_openmp:
                if r[0] == product and r[1] == num_proc:
                    threads_times.append(r[2])  # Время выполнения threads
                    break

        # Отрисовка для OpenMPI
        plt.plot(num_processes, openmpi_times, marker='o', label=f'OpenMPI (продукт {product})')
        # Отрисовка для OpenMP
        plt.plot(num_processes, threads_times, marker='s', label=f'OpenMP (продукт {product})')

    # Общие настройки графика
    plt.xlabel('Количество процессов')
    plt.ylabel('Время выполнения (время в секундах)')
    plt.title('Сравнение времени выполнения между OpenMPI и OpenMP')
    plt.legend()
    plt.grid()
    plt.xticks(num_processes)  # Установить метки по оси X
    plt.savefig('time_execution_all_products.png')
    plt.show()  # Показать график
    plt.close()

def plot_trend(results_openmpi, results_threads):
    # Извлекаем размеры продукции и количество процессов
    products = sorted(set(r[0] for r in results_openmpi))
    num_processes = sorted(set(r[1] for r in results_openmpi))
    
    # Подготовка графика
    plt.figure(figsize=(10, 6))

    # Обработка данных для графика OpenMPI
    for product in products:
        X = np.array(num_processes).reshape(-1, 1)
        y = np.array([r[2] for r in results_openmpi if r[0] == product])
        
        if len(X) == len(y) and len(X) > 0:
            model = LinearRegression()
            model.fit(X, y)
            y_pred = model.predict(X)
            
            plt.plot(num_processes, y_pred, label=f'OpenMPI {product}', linestyle='-', marker=None)

    # Обработка данных для графика Threads
    for product in products:
        X = np.array(num_processes).reshape(-1, 1)
        y = np.array([r[2] for r in results_threads if r[0] == product])
        
        if len(X) == len(y) and len(X) > 0:
            model = LinearRegression()
            model.fit(X, y)
            y_pred = model.predict(X)
            
            plt.plot(num_processes, y_pred, label=f'OpenMPI + OpenMP {product}', linestyle='--', marker=None)

    # Настройки графика
    plt.title('Линии тренда для OpenMPI и OpenMPI + OpenMP')
    plt.xlabel('Количество процессов')
    plt.ylabel('Время выполнения')
    plt.legend()
    plt.grid()
    # plt.show()

def plot_zero_trend(results_openmpi, results_openmp):
    # Извлекаем уникальные количества процессов
    num_processes = sorted(set(r[1] for r in results_openmpi))
    
    avg_openmpi = [0, 0, 0, 0, 0, 0, 0, 0]
    avg_openmp = [0, 0, 0, 0, 0, 0, 0, 0]

    # Расчет средних значений для OpenMPI
    for i in results_openmpi:
        avg_openmpi[i[1] - 1] = (avg_openmpi[i[1] - 1] + i[2]) / 2

    # Расчет средних значений для OpenMP
    for i in results_openmp:
        avg_openmp[i[1] - 1] = (avg_openmp[i[1] - 1] + i[2]) / 2

    # Построение графика
    plt.figure(figsize=(10, 6))
    
    # График для OpenMPI
    plt.plot(num_processes, avg_openmpi, marker='o', label='Среднее OpenMPI (200-20000)', color='b')

    # График для OpenMP
    plt.plot(num_processes, avg_openmp, marker='x', linestyle='--', label='Среднее OpenMPI + OpenMP (200-20000)', color='r')
    
    # Установка заголовков и меток
    plt.title('Сравнение среднего времени выполнения: OpenMPI vs OpenMP')
    plt.xlabel('Количество процессов')
    plt.ylabel('Среднее время выполнения (или другая метрика)')
    plt.legend()
    plt.grid()
    plt.xticks(num_processes)
    plt.show()

if __name__ == "__main__":
    results_openmpi, result_openmp = read_all_information()
    plot_results(results_openmpi, result_openmp)
    plot_performance_metrics(results_openmpi, result_openmp)
    plot_results_all_in_one(results_openmpi, result_openmp)
    plot_trend(results_openmpi, result_openmp)
    plot_zero_trend(results_openmpi, result_openmp)