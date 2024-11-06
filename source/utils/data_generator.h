#pragma once

#include <random> 
#include <vector> 
#include <string> 

std::string 
GenerateWord(std::mt19937& generator
                    , int max_length);


std::vector<std::string> 
GenerateDictionary(std::mt19937& generator
                    , int word_count                   // количество слов
                    , int max_length);                 // количество символов в слове


std::string 
GenerateQuery(std::mt19937& generator
               , const std::vector<std::string>& dictionary
               , int    word_count
               , double minus_prob = 0);


std::vector<std::string> 
GenerateQueries(std::mt19937& generator
              , const std::vector<std::string>& dictionary
              , int query_count                      // количество документов
              , int max_word_count) ;                // количество слов в документе


