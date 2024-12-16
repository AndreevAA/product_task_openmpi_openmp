Реализация метода потенциальных затрат в функции solveProductSubproblem:
Инициализация базисного решения: Используется метод северо-западного угла для получения начального допустимого плана перевозок.
Вычисление потенциалов u и v: На основе базисных клеток рассчитываются потенциалы строк и столбцов.
Расчет оценок ∆: Для небазисных клеток (где поток равен нулю) вычисляются оценки оптимальности.
Проверка оптимальности: Если все ∆ ≥ 0, то текущий план является оптимальным. В противном случае, необходимо улучшать план.
Улучшение плана: В случае отрицательных ∆ необходимо построить цикл и пересчитать потоки по этому циклу. В данном коде этот шаг требуется доработать, так как построение цикла и перераспределение потоков являются трудоемкими процессами.