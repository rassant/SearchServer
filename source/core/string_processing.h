#pragma once

#include <string> 
#include <vector>
#include <set>
#include <string_view>



// вход:   строка слов с пробелами
// внутри: избавляемся от пробелов
// выход:  Вектор слов.
std::vector<std::string_view> 
SplitIntoWords (  std::string_view );



// вход:   контейнер строк 
// внутри: Избавляемся от пустых строк в контейнере.
// выход:  Контейнер set со строками.
template <typename StringContainer>
std::set<std::string,std::less<>> 
MakeUniqueNonEmptyStrings ( const StringContainer &strings )
{
    std::set<std::string,std::less<>> non_empty_strings;
    for ( const std::string_view & str : strings )
    {
        if ( !str.empty( )) {
            non_empty_strings.emplace ( str );
        }
    }
    return non_empty_strings;
}