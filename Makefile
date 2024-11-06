	####################################################################################################
	# make -f MyMakefile для выбора конкретного make файла
	# для копирования файла в буфер обмена
	# cat ./main.cpp|xclip -i -selection clipboard
	# valgrind --leak-check=full ./a.out
	# strip ./a.out он удалит из программы всю отладочную информацию
	####################################################################################################

# CXXFLAGS += -gsplit-dwarf # при оптимизации мы можем увидеть <optimized out> вот что бы этого не было используем этот флаг. Минус он создает дополнительные файлы с информацией для отладки optimized.dwo

# CXXFLAGS += -pg # для работы с параллейными алгоритмами -pg для профилировщика 
				# (замера времени работы функций)после компиляции появиться файл gmon.out  для создания рапорта:
				# gprof ./a.out gmon.out > report.txt
				# файл gmon.out появляется только после запуска и завершения программы

# CXXFLAGS += -ggdb3 # создает более полную отладочную информацию
CXXFLAGS += -O3
CXXFLAGS += -std=c++20 

CXXFLAGS += -Wall -Wextra -Wpedantic  
CXXFLAGS += -pedantic -pedantic-errors
CXXFLAGS += -Werror                         # делает все предупреждения ошибками. Если есть предупреждение код не скомпилируется
CXXFLAGS += -Wfatal-errors                  # останавливает компиляцию после первой же ошибки. т.е выдаст 1 ошибку а не 10
CXXFLAGS += -Wcast-align                    # проверка что бы память была выровнена.
CXXFLAGS += -Wcast-qual                     # проверяет на правильное использование const например константный char (const char*) к указателю на не-константный char (char*)
CXXFLAGS += -Wconversion                    # предупреждать о потери точности
CXXFLAGS += -Wctor-dtor-privacy             # проверяет конструкторы и деструкторы их доступность и ло
CXXFLAGS += -Wduplicated-branches           # проверка дублирования ветвлений
CXXFLAGS += -Wduplicated-cond               # проверка дублирования условий
CXXFLAGS += -Wfloat-equal                   # Предупреждает о проверке на равенство между двумя числами с плавающей точкой.
CXXFLAGS += -Wlogical-op                    # Предупреждает о подозрительных логических выражениях. Например, когда вместо побитового "И" поставили логическое "И"
CXXFLAGS += -Wnon-virtual-dtor              # Предупреждает о том, что у класса есть виртуальные функции-члены, но деструктор при этом не виртуальный
CXXFLAGS += -Wold-style-cast                # Предупреждает о приведении типов в стиле языка C
CXXFLAGS += -Woverloaded-virtual            # Предупреждает о попытке в классе-наследнике перегрузить виртуальную функцию базового класса (забыл добавить overload)
CXXFLAGS += -Wredundant-decls               # предупреждений о ненужных или избыточных объявлениях (декларациях) в программе.
CXXFLAGS += -Wshadow=compatible-local       # не даёт перекрыть локальную переменную другой локальной переменной при условии, что они имеют совместимые типы.
CXXFLAGS += -Wsign-compare                  # предупреждений при сравнении значений разных знаков
CXXFLAGS += -Wsign-conversion               # предотвратить большое количество неявных преобразований,
CXXFLAGS += -Wsign-promo                    # Предупреждает о неочевидном выборе перегруженной функции
CXXFLAGS += -Wzero-as-null-pointer-constant # Предупреждает об использовании целочисленного нуля вместо nullptr
#------------------------------------------------------------------------------------------
LDLIBS += -ltbb -lpthread

SOURCES =  $(wildcard source/core/*.cpp) $(wildcard source/input_output/*.cpp) \
           $(wildcard source/tests/*.cpp) $(wildcard source/utils/*.cpp) *.cpp

all: program
program: 
	@g++ $(CXXFLAGS) $(SOURCES) $(LDLIBS)
	@echo "Компиляция прошла успешна."
