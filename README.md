# cpp-spreadsheet
Дипломный проект: Электронная таблица. Упрощённый вариант существующих электронных таблиц. Ячейки могут быть текстовыми и формульными, а также содержать ссылки на другие ячейки.
## Пример использования:
```cpp
  sheet->SetCell("A1"_pos, "2");
  sheet->SetCell("A2"_pos, "=A1+1");
  sheet->SetCell("A3"_pos, "=A2+2");
  sheet->SetCell("A4"_pos, "=A3+3");
  sheet->SetCell("A5"_pos, "=A1+A2+A3+A4");

  auto* cell_A5_ptr = sheet->GetCell("A5"_pos);
  ASSERT_EQUAL(std::get<double>(cell_A5_ptr->GetValue()), 18);
```
Другие примеры использования можно найти в файле [main.cpp](https://github.com/dea-a/cpp-spreadsheet/blob/main/spreadsheet/main.cpp)
## Особенности
- Реализована поддержка исключений:
```
#REF!    - Cсылка на ячейку с некорректной позицией
#VALUE! -  Ячейка не может быть трактована как число
#DIV/0! -  Если делитель равен 0, значение ячейки — ошибка
```
Если формула зависит от нескольких ячеек, каждая из которых содержит ошибку вычисления, результирующая ошибка может соответствовать любой из них.
- Реализована проверка ячеек на циклические зависимости. Программа должна кидать исключение CircularDependencyException, а ячейка не должна меняться.
Пример:
```cpp
sheet->SetCell("A1"_pos, "=A2");
sheet->SetCell("A2"_pos, "=A1");
```
## Использованные идеомы, технологии и элементы языка
- OOP: inheritance, abstract interfaces, final classes
- STL smart pointers
- std::variant and std:optional
- ANTLR для генерации лексического и семантического анализаторов
- CMake 
## Сборка
- Для сборки понадобится генератор ANTLR. 
- Правила грамматики находятся в файле Formula.g4. 
- Предоставлены файлы CMakeLists.txt и FindANTLR.cmake.
