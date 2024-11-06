
#pragma once
#include <future> 
#include <execution> 
#include <mutex> 
#include <functional> 
#include <deque> 
#include <pstl/glue_execution_defs.h>
#include <vector> 
#include <set> 
#include <string> 
#include <string_view>
#include <map> 
#include <algorithm> 
#include "document.h"
#include "../input_output/read_input_functions.h"
#include "../core/string_processing.h"
#include "../core/concurrent_map.h"


// максимально количество документов которое будет выдано в качестве результата при поиске.
const int MAX_RESULT_DOCUMENT_COUNT = 5;


class SearchServer {
public:


// конструктор
// вход:   контейнер строк ( vector, set )для стоп слов
// внутри: проверка что входящие данные не содержат запрещенные символы от 0 до 31
// выход:  стоп слова
   template < typename StringContainer >
   explicit 
   SearchServer ( const StringContainer& stop_words );


// конструктор
// Вход:   строка для стоп слов
// Внутри: Вызов конструтора для работы с контейнером ( vector, set )
// выход:  стоп слова
   explicit 
   SearchServer ( const std::string_view stop_words_text );

   explicit 
   SearchServer ( const std::string& stop_words_text );

// вход:   строка
// внутри: помещение в приватное поле контейнера  const set < string > stop_words_ стоп слов.  // выход:  -
   void 
   SetStopWords ( const std::string_view text );


// Сколько было найдено документов
// вход:   -
// внутри: -
// выход:  количество найденных документов которые содержат хотя бы одно слово из поискового запроса.
   int 
   GetDocumentCount ( ) const ;


// основной метод для добавления документов
// Вход:
// 1 )уникальный номер документа ID
// 2 )Содержимое документа в виде непрерывной строки
// 3 )Актуальность документа (статус) Актуальный, отбракованный, заблокирован, удален
// 4 )Рейтинг 
// Внутри: 
//         Проверка: уникальный ID, разрешенные символы, 
//         Расчет важности каждого слова в документе TF
//         исключаются (отбрасываются ) документы содержащие минус слова
//         заполнение map < int, DocumentData > documents_; ( ID, (рейтинг, статус) )
//         заполнение map < int, int > n_doc_id_ ; (очередность добавления документа, ID )
// выход:  -
   void 
   AddDocument ( int  document_id, std::string_view document, 
                       DocumentStatus  status, const std::vector < int > &ratings );


// Выдает самые подходящие документы, согласно поисковому запросу.
// Вход:   поисковая строка запроса
//         Акутульный статус документа где должен происходить поиск.
// Внутри: вызов основного метода FindTopDocuments
// Выход:  контейнер документов ( id, релеванс, рейтинг )
   
   std::vector < Document > 
   FindTopDocuments ( const std::string_view raw_query, 
                       DocumentStatus status = DocumentStatus::ACTUAL ) const;

// ----------------------------------------------------------------------------------------

std::vector < Document >
FindTopDocuments ( const std::execution::parallel_policy&, const std::string_view raw_query,
                   DocumentStatus status = DocumentStatus::ACTUAL ) const;


template < typename ExecutionPolicy >
std::vector < Document >
FindTopDocuments ( const ExecutionPolicy &, const std::string_view raw_query,
                   DocumentStatus status = DocumentStatus::ACTUAL ) const
{
    return FindTopDocuments (raw_query
            , [status]( int, DocumentStatus s, int )
            { return status == s;}
            );
}


template < typename Predicate, typename ExecutionPolicy >
std::vector < Document >
FindTopDocuments (const ExecutionPolicy &, const std::string_view raw_query, Predicate predicate ) const
{
    Query query_words;
    query_words = ParseQuery ( raw_query, false );
    std::vector < Document > matched_documents;

    if (std::is_same<std::decay_t<ExecutionPolicy>, std::execution::parallel_policy>::value)
    {
        matched_documents = SetDocumentForRelevance ( FindAllDocuments ( std::execution::par, query_words, predicate ));
    }
    else
    {
        matched_documents = SetDocumentForRelevance ( FindAllDocuments ( query_words, predicate ));
    }
    SetCurrentSizeForRezult ( matched_documents, MAX_RESULT_DOCUMENT_COUNT );

    return matched_documents;
}
// ----------------------------------------------------------------------------------------





// Выдает самые подходящие документы, согласно поисковому запросу.
// Вход:   строка, шаблон
// Внутри: проверка на отсутствие документов содежащие минус слова.
// Выход:  только релевансные документы и не более чем MAX_RESULT_DOCUMENT_COUNT
   template < typename Predicate >
   std::vector < Document > 
   FindTopDocuments ( const std::string_view raw_query, Predicate predicate ) const ;



// Проверяет есть ли совподение документа с поисковым запросом.
// Вход:   Строка и id
// Внутри: Проверка на минус слова, и на корректность ввода символов.
// Выход:  <документ в виде строки и статус>
   std::tuple < std::vector < std::string_view >, DocumentStatus > 
   MatchDocument ( const std::string_view raw_query, int document_id ) const ;

   std::tuple < std::vector < std::string_view >, DocumentStatus > 
   MatchDocument ( const std::execution::sequenced_policy&, const std::string_view raw_query, int document_id ) const ;

   std::tuple < std::vector < std::string_view >, DocumentStatus > 
   MatchDocument ( const std::execution::parallel_policy&, const std::string_view raw_query, int document_id ) const ;
    auto begin ( )
    {
        return SearchServer::n_doc_id_.begin( );
    }

    auto end ( )
    {
        return SearchServer::n_doc_id_.end( );
    }


    const std::map < std::string_view, double >& 
    GetWordFrequencies ( int document_id ) const;

    void RemoveDocument ( int document_id );
    void RemoveDocument ( const std::execution::sequenced_policy&, int document_id );
    void RemoveDocument ( const std::execution::parallel_policy&, int document_id );
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    struct Query
    {
        std::vector < std::string_view > minus_words;
        std::vector < std::string_view > plus_words;
    };
    struct QueryWord 
    {
        std::string_view word;
        bool is_minus;
        bool is_stop;
    };
    std::map < std::string_view, std::map < int, double >> word_to_document_freqs_; // Слово, ID документа где это слово встречается, рейтинг TF слова в документе.
    std::map < int, std::map < std::string_view, double >> document_to_word_freqs_; // id <слово, рейтинг>
                                                                               
    std::map < int, DocumentData > documents_;    // ID, < рейтинг, статус >
    std::set < int > n_doc_id_;     // пара номер введеного документа / ID

    // основные контейнеры для хранения данных должны быть string
    std::set < std::string, std::less<> > stop_words_;   // стоп слова. Документы их содержащие будут исключены из результатов
    std::deque < std::string > storage_;                   // контейнер, (буфер) для хранения содержимого документов.


    void
    SortAndRemoveDublicate ( std::vector < std::string_view >& ) const;

// Вычисляем рейтинг TF для одного слова в документе.
// вход:  количество слов в документе.
// Выход: рейтинг для одного слова в документе. 
   double 
   CalculateTF ( const double total_words ) const ;

    
// Определяем как часто встречается слово в документе.
// Вход:   количество документов где встречается слово.
// Внутри: логарифм от (Общее количество документов / к-во документов где встречается слово )
// выход:  полезность слова. Чем в большем количестве документов есть это слово тем ниже IDF
   double 
   CalculateIDF ( const double match_word_in_docs ) const ;


// стоп слова это слова которые не участвуют в поиске.
// это такие слова как: о, а, об, у, на, в и так далее
// вход:  слово которое необходимо проверить является ли оно стоп словом или нет.
// выход: true - является стоп словом.
   bool 
   IsStopWord ( const std::string_view word ) const ;


// Разбиваем строку на слова убеждаемся что это не стоп слово и помещаем его в контейнер
// Вход:  строка слов
// Выход: контейнер слов без стоп слов.
   std::vector < std::string_view > 
   SplitIntoWordsNoStop ( const std::string_view text ) const ;


// Проверяем что строка не содежит запрещенных символов.
   static bool 
   IsValidWord ( const std::string_view word );


// задаем характеристику слова.(это минус или стоп слово)
// Вход:   слово
// Внутри: если это минус слово, отбрасываем знак минус.
// Выход:  Структура с информацией ( string, true, false )[ Слово. это минус слово, это стоп слово ]
   QueryWord 
   ParseQueryWord ( std::string_view word ) const;


// разбиваем строку запроса на плюс и минус слова
// вход : Строка запроса.
// выход: контейнеры с минус и плюс словами.
   Query 
   ParseQuery ( const std::string_view text, bool is_parallel ) const ;


// Вычисление среднего рейтинга
// Вход: контейнер оценок рейтинга
// Выход: Средний рейтинг.
   static int 
   ComputeAverageRating ( const std::vector < int > & ratings );


// удаление документов по id в tf_idf если были встречены минус слова.
// Вход:  минус слова и TF IDF 
//        Изменение tf_idf !
// Выход: нет.
//        Удаление во входном контейнере tf_idf
   template < typename ExecutionPolicy >
   void 
   EraseTFIDFwithMinusWords ( const ExecutionPolicy &, 
                              const std::vector < std::string_view > &minus_words, 
                              ConcurrentMap < int, double > &tf_idf ) const;


// расчитываем TF IDF
// Вход: строка плюс слов. Предикат  [status]( int, DocumentStatus s, int ){ return status == s;} 
// Выход: пара id документа и TF IDF для этого документа.
   template < typename Predicate, typename ExecutionPolicy >
   void
   SetTFIDFwithWordPlus ( const ExecutionPolicy &policy
                        , const std::vector < std::string_view >& words
                        , ConcurrentMap  < int, double > & tf_idf
                        , Predicate predicate ) const;


// вход: Плюс слова, id документа
// Выход: Если слов есть в этом документе помещаем его в vector < string > 
   std::vector < std::string_view > 
   AddPlusWords ( const std::vector < std::string_view > &plus_words, const int document_id ) const;


// Вход: минус слова, id документа, плюс слова которые были найдены в документе.
// Внутри: Если минус слово есть в документе, то удалить все слова из вектора vector < string > &matched_words 
   void 
   ClearResultWithMinusWords ( const std::vector < std::string_view > &minus_words, const int document_id, 
                                     std::vector < std::string_view > &matched_words ) const;


// вход: контейнер пара id документа и его TF IDF
// Выход:контейнер  заполненый структурой Document id, relevance, rating 
   std::vector < Document > 
   ResultMatchedDocuments ( ConcurrentMap < int, double > &tf_idf ) const;


// Вход:   Поисковая строка, предикат описывающий статус документа
// Внутри: Удалить из результата все документы с минус словами.
// Выход:  Пара ID документа и его релеванс ( TF IDF )
   template < typename Predicate >
   std::vector < Document > 
   FindAllDocuments ( const Query& query_words, Predicate predicate ) const ;

   /* template < typename Predicate > */
   /* std::vector < Document > */ 
   /* FindAllDocuments ( const std::execution::parallel_policy& */
   /*, const Query& query_words, Predicate predicate ) const ; */

   template < typename Predicate, typename ExecutionPolicy >
   std::vector < Document > 
   FindAllDocuments ( const ExecutionPolicy &
                    , const Query& query_words, Predicate predicate ) const ;

// Вход:  контейнер документов (уже очищенный от документов с минус словами и т.д. )
// Внутри: Документы сортируются по релеванс ( TF IDF )
// inaccuracy необходимо для сравнения double величин. abc - это модуль
// при практически одинаковой релевантности сортировка идет по рейтингу
// Выход:  Отсортированный контейнер документов (по релеванс TF IDF )лоЖц
   std::vector < Document > 
   SetDocumentForRelevance (std::vector < Document > matched_documents ) const;


// Вход: Контейнер наденных документов, максимальное количество документов которое должно быть в результате
// Внутри: Обрезается контейнер до нужного количества ( max_result_document_count )
   void 
   SetCurrentSizeForRezult ( std::vector < Document > &matched_documents, 
                                   const long unsigned int max_result_document_count ) const;


// проверка.
// id не отрицательный. id не повторялся.
   void 
   CheckIdDocument ( const int document_id ) const;


// проверка что строка не содержит спец символы
   void 
   CheckCharInWord ( const std::string_view text ) const ;

   template < typename T >
   void ParSeqRemoveDocument ( const T &police, int document_id )
   {
           std::vector < const std::string_view *> 
           words_to_remove ( document_to_word_freqs_[document_id].size( ));
           
 
           // Получаем список слов, присутствующих в удаляемом документе
           std::transform ( police
, document_to_word_freqs_[document_id].begin( )
, document_to_word_freqs_[document_id].end( )
, words_to_remove.begin( )
, []( const auto& word_freq )
                            {
                             return &word_freq.first;
                            } 
           ); // std::transform 
           
 
           // Удаляем упоминания этих слов из word_to_document_freqs_
           std::for_each ( police
, words_to_remove.begin( )
, words_to_remove.end( )
, [&]  ( const std::string_view * wordPtr )
                          {
                             word_to_document_freqs_[* wordPtr].erase( document_id );
                          } 
           );// std::for_each 
             
           // Удаляем информацию о документе из остальных контейнеров
           n_doc_id_.erase( document_id );
           documents_.erase( document_id );
           document_to_word_freqs_.erase( document_id );
   }


};// class SearchServer
  
// -------------------------------------------------------------------------------------------


template < typename StringContainer > SearchServer::
SearchServer ( const StringContainer& stop_words )
             : stop_words_( MakeUniqueNonEmptyStrings ( stop_words )) 
{ 
    if ( ! std::all_of ( stop_words_.begin( ), stop_words_.end( ), IsValidWord ))
    {
         throw std::invalid_argument ( "не допустимый диапазон символов от 0 до 31" );
    }
}



template < typename Predicate >
std::vector < Document > SearchServer::
FindTopDocuments ( const std::string_view raw_query, Predicate predicate ) const 
{
    Query query_words = ParseQuery ( raw_query, false );
    std::vector < Document > matched_documents = 
        SetDocumentForRelevance ( FindAllDocuments ( query_words, predicate ));

    SetCurrentSizeForRezult ( matched_documents, MAX_RESULT_DOCUMENT_COUNT );

    return matched_documents;
}// FindTopDocuments



template < typename Predicate, typename ExecutionPolicy >
void SearchServer::
SetTFIDFwithWordPlus ( const ExecutionPolicy &
                     , const std::vector < std::string_view > &words
                     , ConcurrentMap  < int, double > & tf_idf
                     , Predicate predicate ) const
{

    if (std::is_same<std::decay_t<ExecutionPolicy>, std::execution::parallel_policy>::value)
    {
        for_each ( std::execution::par, words.begin( ), words.end( )
        ,[&]( const std::string_view word )
        {
            if ( word_to_document_freqs_.count( word ) > 0 )
            {
                size_t match_word_in_docs = word_to_document_freqs_.at( word ).size( );
                double idf = CalculateIDF ( static_cast < double > ( match_word_in_docs ));

                for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word ))
                {
                    if ( predicate ( document_id 
                        , documents_.at( document_id ).status
                        , documents_.at( document_id ).rating ))
                    {
                        tf_idf[document_id].ref_to_value += idf * TF;
                    }
                }
            }
        } ); // for_each
    }
    else
    {
        for_each ( words.begin( ), words.end( )
        ,[&]( const std::string_view word )
        {
            if ( word_to_document_freqs_.count( word ) > 0 )
            {
                size_t match_word_in_docs = word_to_document_freqs_.at( word ).size( );
                double idf = CalculateIDF ( static_cast < double > ( match_word_in_docs ));

                for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word ))
                {
                    if ( predicate ( document_id 
                        , documents_.at( document_id ).status
                        , documents_.at( document_id ).rating ))
                    {
                        tf_idf[document_id].ref_to_value += idf * TF;
                    }
                }
            }
        } ); // for_each

    }




}



template < typename ExecutionPolicy >
void SearchServer::
EraseTFIDFwithMinusWords ( const ExecutionPolicy & 
                        , const std::vector < std::string_view > &minus_words
                        , ConcurrentMap < int, double > &tf_idf ) const
{
     if (std::is_same<std::decay_t<ExecutionPolicy>, std::execution::parallel_policy>::value)
     {
        for_each (std::execution::par, minus_words.begin( ), minus_words.end( )
            ,[&]( const std::string_view word )
            {
                if ( word_to_document_freqs_.count( word ) > 0 )
                {
                    for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word ))
                    {
                        tf_idf.erase( document_id );
                    }
                }
            } );

     }
     else
     {
    for_each ( minus_words.begin( ), minus_words.end( )
            ,[&]( const std::string_view word )
            {
                if ( word_to_document_freqs_.count( word ) > 0 )
                {
                    for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word ))
                    {
                        tf_idf.erase( document_id );
                    }
                }
            } );

     }
}


template < typename Predicate >
std::vector < Document > SearchServer::
FindAllDocuments ( const Query& query_words, Predicate predicate ) const 
{
    std::map < int, double > tf_idf ;

    /* for ( const std::string_view word: query_words.plus_words) */
    /* { */
    /*     if ( word_to_document_freqs_.count( word )== 0 ) */
    /*     { */
    /*         continue; */
    /*     } */

    /*     auto match_word_in_docs = word_to_document_freqs_.at( word ).size( ); */
    /*     double idf = CalculateIDF ( static_cast<double> (match_word_in_docs) ); */

    /*     for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word )) */
    /*     { */
    /*         if ( predicate ( document_id */ 
    /*            , documents_.at( document_id ).status */
    /*            , documents_.at( document_id ).rating )) */
    /*         { */
    /*             tf_idf[document_id] += idf * TF; */
    /*         } */
    /*     } */
    /* }// for plus_words */
     

    /* for ( const auto& word :query_words.minus_words ) */
    /* { */
    /*     if ( word_to_document_freqs_.count( word )== 0 ) */
    /*     { */
    /*         continue; */
    /*     } */

    /*     for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word )) */
    /*     { */
    /*         tf_idf.erase( document_id ); */
    /*     } */
    /* } // for minus_words */

    for_each ( query_words.plus_words.begin( ),query_words.plus_words.end( )
    ,[&]( const std::string_view &word )
    {
        if ( word_to_document_freqs_.count( word ) > 0 )
        {
            size_t match_word_in_docs = word_to_document_freqs_.at( word ).size( );
            double idf = CalculateIDF ( static_cast < double > ( match_word_in_docs ));

            for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word ))
            {
                if ( predicate ( document_id 
                    , documents_.at( document_id ).status
                    , documents_.at( document_id ).rating ))
                {
                    tf_idf[document_id] += idf * TF;
                }
            }
        }
    } ); // for_each
    for_each ( query_words.minus_words.begin( ), query_words.minus_words.end( )
            ,[&]( const std::string_view &word )
            {
                if ( word_to_document_freqs_.count( word ) > 0 )
                {
                    for ( const auto& [document_id, TF] : word_to_document_freqs_.at( word ))
                    {
                        tf_idf.erase( document_id );
                    }
                }
            } );

    std::vector<Document> matched_documents;
    for ( const auto&[document_id, relevance] : tf_idf )
    {
        matched_documents.push_back(
                { document_id, relevance, documents_.at( document_id ).rating } );
    }
    return matched_documents;

}// FindAllDocuments


template < typename Predicate, typename ExecutionPolicy >
std::vector < Document > SearchServer::
FindAllDocuments ( const ExecutionPolicy& policy
                , const Query& query_words, Predicate predicate ) const 
{
    if (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::parallel_policy>)
    {
        ConcurrentMap  < int, double >  tf_idf ( query_words.plus_words.size() ); 
        SetTFIDFwithWordPlus     ( policy, query_words.plus_words, tf_idf, predicate );
        EraseTFIDFwithMinusWords ( policy, query_words.minus_words, tf_idf );

        return ResultMatchedDocuments ( tf_idf );
    }

    return FindAllDocuments ( query_words, predicate ); 
}
