#include "./source/tests/test_speed.h"
/*#include "data_generator.h"*/

void Example ()
{
  using namespace std;
  SearchServer search_server1("and with"s); // слова которые должны быть исключены из документов
  int id = 0;// номер документа
  for ( const string& text : {   // список документов где будет поиск
          "white cat and yellow hat"s,
          "curly cat curly tail"s,
          "nasty dog with big eyes"s,
          "nasty pigeon john"s, }
  ) {
    search_server1.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2}); // добавляем документы в базу
  } //for
  cout<<endl << "По умолчанию документы имеют статус: ACTUAL:"s << endl;
  
  // поиск документов используя парраллейную версию
  for (const Document& document : search_server1.FindTopDocuments(execution::par, "curly nasty cat"s)) {
    PrintDocument(document);
  }

  cout << "Документы имеющие статус: BANNED:"s << endl;
  // Поиск документов имеющих статус BANNED используя последовательную версию
  for (const Document& document : search_server1.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
    PrintDocument(document);
  }

  cout << "Выводим документы имеющие только четные индификаторы (id):"s << endl;
  // параллельная версия
  for (const Document& document : search_server1.FindTopDocuments
                      (execution::par, "curly nasty cat"s
                     , [](int document_id, DocumentStatus , int ) { return document_id % 2 == 0; })
  ){
    PrintDocument(document);
  } //for
}
