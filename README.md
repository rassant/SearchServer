# Поисковый сервер # 


### Описание ###
Этот проект представляет собой поисковый сервер, 
который осуществляет эффективный поиск слов в документе с учетом стоп-слов и релевантности. 

Сборка проекта может быть, осуществлена двумя путями:
    * make (g++) 
    * cmake (clang) вариант c clang на 30% быстрее. Как в последовательных так и в параллейных вычислениях.

### Функционал ###
Сервер выдает результат поиска документов равный MAX_RESULT_DOCUMENT_COUNT {5}

### Mетод для наполнения базы документов:

    void AddDocument (       int  document_id, 
                     string_view  document, 
                  DocumentStatus  status, 
              const vector <int>  &ratings ){...};
Аргументы метода:
            document_id - уникальный номер документа
            document    - текст документов
            status      - статус документа ACTUAL,IRRELEVANT, BANNED, REMOVED, 
            ratings     - вектор рейтингов документа 

### Основной метод для поиска документов:

    template < typename Predicate, typename ExecutionPolicy >
    std::vector < Document >
    FindTopDocuments ( const ExecutionPolicy  &execution 
                      const std::string_view  raw_query, 
                                   Predicate  predicate ) const {...};
                
Аргументы метода :
            execution - параллейный или последовательный поиск документов
            raw_query - пользовательский запрос
            predicate - формат поиска. 

Возвращает: релевантные документы содержащие: 
            номер документа (id), 
            точность совпадения (relevance), 
            оценка (rating);


### Тесты ###
для проверки сервера использовались тесты из файла tests_SearchServer.cpp
1) Проверка что стоп слова были исключены из документа (например в, на, об, а, о и тд)
2) Из результата поиска были исключены все документы которые содержат минус слова. 
    (например не показывать в результатах поиска документы котоыре содержат слово 'кот' ) 
3) найденные документы отсортированы по убыванию реливантности.
4) Проверка ретинга выбранного документа. 
5) Проверка иного вывода запроса сервера за счет предиката в FindTopDocuments
6) Проверка релевантности
7) Проверка правильности нахождения документа
8) Количество найденных документов. 
Так же тест сравнения скорости при параллейном и последовательном поиске (test_speed.h)


### Структура проекта ###
```
search-server/
├── source/
│   ├── core/
│   │   ├── concurrent_map.h        [для работы со словарем в парралейном режиме.]
│   │   ├── document.h              [представление документа]
│   │   ├── document.cpp
│   │   ├── paginator.h             [Вывод результата поиска постранично.]
│   │   ├── process_queries.h       [распараллеливает поисковый запрос]
│   │   ├── process_queries.cpp
│   │   ├── request_queue.h         [статистика. раз в сутки. Сколько было запросов, на которые ничего не нашлось]
│   │   ├── request_queue.cpp
│   │   ├── search_server.h         [Основной класс для осуществления поиска]
│   │   ├── search_server.cpp
│   │   ├── string_processing.h     [подготовка строки, удаление пробелов, пустых строк]
│   │   └── string_processing.cpp
│   ├── input_output/
│   │   ├── read_input_functions.h  [чтение строки]
│   │   └── read_input_functions.cpp
│   ├── utils/
│   │   ├── log_duration.h          [вспомогательный класс для замера производительности]
│   │   ├── data_generator.cpp
│   │   └── data_generator.h        [Генерация данных для тестов]
│   └── test/                       [тесты]
│       ├── test_example_functions.h[тесы SearchServer]
│       ├── test_example_functions.cpp
│       ├── test_framework.h        [Набор инструментов для модульных тестов]
│       ├── test_speed.h            [Замер скорости работы]
│       └── test_speed.cpp
├── CMakeLists.txt
├── main.cpp
├── example.cpp
├── example.h
└── README.md
```
