#include <cmath> 
#include <iterator> 
#include <execution>
#include "search_server.h"
#include "string_processing.h"
#include "document.h"
#include <iterator> 
#include <string> 
#include <numeric> 
#include <tuple> 

using namespace std;

SearchServer::
SearchServer ( const string_view stop_words_text ): SearchServer ( SplitIntoWords ( stop_words_text ))
{ 
    CheckCharInWord ( stop_words_text );
}

SearchServer::
SearchServer ( const string& stop_words_text ): SearchServer ( SplitIntoWords ( stop_words_text ))
{ 
    CheckCharInWord ( stop_words_text );
}

void SearchServer::
SetStopWords ( const string_view text )
{
    for ( const string_view word : SplitIntoWords ( text )) 
    {
        stop_words_.emplace( word );
    }
}


int SearchServer::
GetDocumentCount ( ) const 
{
    return static_cast < int > ( documents_.size( ));
}


void SearchServer::
AddDocument ( int  document_id, 
              string_view  document, 
              DocumentStatus  status, 
              const vector < int >  &ratings )
{
    storage_.emplace( storage_.end( ), document );

    CheckIdDocument ( document_id );
    CheckCharInWord ( storage_.back( ));

    const vector < string_view > words = SplitIntoWordsNoStop ( storage_.back( ));
    double tf = CalculateTF ( static_cast < double > ( words.size( )) );

    for ( string_view word: words )
    {
        word_to_document_freqs_[word][document_id] += tf ;
        document_to_word_freqs_[document_id][word] += tf ;
    }    

    documents_.emplace( document_id, DocumentData{ ComputeAverageRating ( ratings ), status} );

    n_doc_id_.insert( document_id );

}// AddDocument


vector < Document > SearchServer::
FindTopDocuments ( const string_view raw_query, DocumentStatus status ) const 
{
    return FindTopDocuments ( raw_query, [status]( int, DocumentStatus s, int ){ return status == s;} );
}

std::vector < Document > SearchServer::
FindTopDocuments ( const std::execution::parallel_policy&, const std::string_view raw_query,
                   DocumentStatus status  ) const
{
    return FindTopDocuments ( std::execution::par, raw_query
            , [status]( int, DocumentStatus s, int )
            { return status == s;}
            );
}

tuple < vector < string_view >, DocumentStatus > SearchServer::
MatchDocument ( const string_view raw_query, int document_id ) const
{
      if ( document_to_word_freqs_.count( document_id )== 0 )
     {
         throw std::out_of_range ( "Документ с таким ID не найден" );
     }
    const Query query = ParseQuery ( raw_query, false );

    /* for ( const string_view word : query.minus_words ){ */
    /*     if ( word_to_document_freqs_.count( word ) == 0 ){ */
    /*         continue; */
    /*     } */
    /*     if ( word_to_document_freqs_.at( word ).count( document_id )) { */
    /*         return { std::vector < std::string_view >{}, documents_.at( document_id ).status}; */
    /*     } */
    /* } */
    
    vector<string_view> matched_words = AddPlusWords ( query.plus_words, document_id );
    ClearResultWithMinusWords ( query.minus_words, document_id, matched_words );

    return { matched_words, documents_.at( document_id ).status };
}




tuple < vector < string_view >, DocumentStatus > SearchServer::
MatchDocument ( const std::execution::sequenced_policy&, const std::string_view raw_query, int document_id ) const
{
    return MatchDocument ( raw_query, document_id );
}
//******************************************************************************************
//******************************************************************************************

tuple < vector < string_view >, DocumentStatus > SearchServer::
MatchDocument ( const std::execution::parallel_policy& policy
            , const std::string_view raw_query, int document_id ) const
{
    if ( document_to_word_freqs_.count( document_id ) == 0 )
    { 
        throw std::out_of_range ( "Документ с таким ID не найден" ); 
    }
    const Query query = ParseQuery ( raw_query, true );
    vector < string_view > matched_words;

    if ( std::any_of ( policy, query.minus_words.begin( ), query.minus_words.end( ),
        [&]( const std::string_view minus_word )
        {
             if ( word_to_document_freqs_.count( minus_word ) > 0 )
             {
                  return word_to_document_freqs_.at( minus_word ).count( document_id );
             }
             return static_cast < size_t > ( 0 );
        } ))
    {
        return { matched_words, documents_.at( document_id ).status };
    }

    matched_words.resize( query.plus_words.size( ));

    auto end_copy = std::copy_if ( policy, query.plus_words.cbegin( ), query.plus_words.cend( ), matched_words.begin( ),
                   [&]( const string_view plus_word )
                   {
                        if ( word_to_document_freqs_.count( plus_word ) > 0
                           //  && !plus_word.empty( )
                            )
                        {
                             return word_to_document_freqs_.at( plus_word ).count( document_id );
                        }
                        return static_cast < size_t > ( 0 );
                   } );

    std::sort ( matched_words.begin( ), end_copy );
    auto solo = std::unique ( matched_words.begin( ), end_copy );
    matched_words.erase( solo, matched_words.end( ));

    return { matched_words, documents_.at( document_id ).status };
}
//******************************************************************************************
//******************************************************************************************
double SearchServer::
CalculateTF ( const double total_words ) const 
{
    return static_cast < double > ( 1.0 )/ total_words;
}


double SearchServer::
CalculateIDF ( const double match_word_in_docs ) const 
{
    return  std::log ( static_cast < double > ( n_doc_id_.size( )) / match_word_in_docs );
}


bool SearchServer::
IsStopWord ( const string_view word ) const 
{
    return stop_words_.count( word ) > 0;
}


vector < string_view > SearchServer::
SplitIntoWordsNoStop ( const string_view text ) const 
{
    vector < string_view > words;
    for ( const string_view word : SplitIntoWords ( text )) 
    {
        if ( !IsStopWord ( word )) {
            words.push_back( word );
        }
    }
    return words;
}


bool SearchServer::
IsValidWord ( const string_view word )
{
    return none_of ( word.begin( ), word.end( ), []( char c ){
            return c >= '\0' && c < ' ';
            } );
}


SearchServer::QueryWord SearchServer::
ParseQueryWord ( string_view word ) const
{
    bool is_minus = false;
    if ( word[0] == '-' )
    {
        is_minus = true;
        word = word.substr( 1 );
    }
    CheckCharInWord ( word );

    if ( word.empty( )|| word[0] == '-' )
        throw invalid_argument ( "не вверный ввод символа -" );

    return { word, is_minus, IsStopWord ( word )};
}


SearchServer::Query SearchServer::
ParseQuery ( const string_view text, bool is_parallel ) const 
{
    CheckCharInWord ( text );
    Query query;

    if ( text.empty( ))
    {
        throw invalid_argument ( "попытка добавить пустой документ" );
    }

    for ( const string_view word : SplitIntoWordsNoStop ( text )) 
    {
        const QueryWord query_word = ParseQueryWord ( word );

        if ( ! query_word.is_stop )
        {
            if ( query_word.is_minus )
            {
                query.minus_words.push_back( query_word.word );
            }
            else
            {
                query.plus_words.push_back( query_word.word );
            }
        }
    }

    if ( is_parallel == false )
    {
        SortAndRemoveDublicate ( query.minus_words );
        SortAndRemoveDublicate ( query.plus_words );
    }
    return query;
}

void SearchServer::
SortAndRemoveDublicate ( std::vector < std::string_view >& v ) const
{
        sort ( v.begin( ), v.end( ));
        auto temp = std::unique ( v.begin( ), v.end( ));
        v.erase( temp, v.end( ));
}

int SearchServer::
ComputeAverageRating ( const vector < int > & ratings )
{
    if ( ratings.empty( )) {
        return 0;
    }
    int rating_sum = accumulate ( ratings.begin( ), ratings.end( ), 0 );

    return rating_sum / static_cast < int > ( ratings.size( ));
}


vector < string_view > SearchServer::
AddPlusWords ( const vector < string_view > &plus_words, const int document_id ) const
{
    vector < string_view > matched_words;

    for ( const string_view word : plus_words )
    {
        if ( word_to_document_freqs_.count( word ) == 0 )
        {
            continue;
        }

        if ( word_to_document_freqs_.at( word ).count( document_id )) 
        {
            matched_words.push_back( word );
        }
    }// for
    return matched_words;
}


void SearchServer::
ClearResultWithMinusWords ( const vector < string_view > &minus_words, const int document_id, 
        vector < string_view > &matched_words ) const
{
    for ( const string_view word : minus_words )
    {
        if ( word_to_document_freqs_.count( word ) == 0 )
        {
            continue;
        }

        if ( word_to_document_freqs_.at( word ).count( document_id )) 
        {
            matched_words.clear( );
            break;
        }
    }
}


vector < Document > SearchServer::
ResultMatchedDocuments ( ConcurrentMap < int, double > &tf_idf ) const
{
    vector < Document > matched_documents;
    for ( const auto&[document_id, relevance] : tf_idf.BuildOrdinaryMap() )
    {
        matched_documents.push_back( 
                { document_id, relevance, documents_.at( document_id ).rating } );
    }
    return matched_documents;
}


vector < Document > SearchServer::
SetDocumentForRelevance ( vector < Document > matched_documents ) const
{
    const double inaccuracy = 1e-6;

    sort ( matched_documents.begin( ), matched_documents.end( ), 
           [inaccuracy]( const Document& lhs, const Document& rhs )
           {
                if ( abs ( lhs.relevance - rhs.relevance ) < inaccuracy )
                {
                    return lhs.rating > rhs.rating;  // при одинаковой релевантности сортировка идет по рейтингу
                } 
                else 
                {
                    return lhs.relevance > rhs.relevance;
                }
           } );// sort

    return matched_documents;
}


void SearchServer::
SetCurrentSizeForRezult ( vector < Document > &matched_documents, const long unsigned int max_result_document_count ) const
{
    if ( static_cast < long unsigned int > ( matched_documents.size( )) > max_result_document_count )
    {
        matched_documents.resize( max_result_document_count );
    }
}


void SearchServer::
CheckIdDocument ( const int document_id ) const 
{
    if ( document_id < 0 )
    {
        throw invalid_argument ( "id не может быть отрицательным" );
    }

    if ( n_doc_id_.find( document_id )!= n_doc_id_.end( ))
    {
        throw invalid_argument ( "Ошибка ID документов, совпадают" );
    }
}


void SearchServer::
CheckCharInWord ( const string_view text ) const {
    if ( ! IsValidWord ( text ))
    {
        throw invalid_argument ( "не допустимый диапазон символов от 0 до 31" );
    }
}


const map < string_view, double >& SearchServer::
GetWordFrequencies ( int document_id ) const
{
    if ( document_to_word_freqs_.count( document_id ) == 0 )
    { 
        throw std::out_of_range ( "Документ с таким ID не найден" ); 
    }
    return document_to_word_freqs_.at( document_id );
}


void SearchServer::
RemoveDocument ( int document_id )
{
    RemoveDocument ( std::execution::seq, document_id );
}

void SearchServer::
RemoveDocument ( const std::execution::sequenced_policy&, int document_id )
{
    ParSeqRemoveDocument ( std::execution::seq, document_id );
}
 
void SearchServer::
RemoveDocument ( const std::execution::parallel_policy&, int document_id )
{
    ParSeqRemoveDocument ( std::execution::par, document_id );
}
