#include "document.h"
#include <string> 
#include <string_view>
#include <vector>
#include <iostream>
#include "string_processing.h"






std::vector<std::string_view> 
SplitIntoWords ( std::string_view text )
{
    std::vector<std::string_view> words;
    while  ( true )
    {
        const auto space = text.find(' ');
        words.push_back(text.substr(0,space));
        if  ( space == text.npos )
        {
            break;
        }
        else
        {
            text.remove_prefix (space + 1);
        }
    }
    return words;
}