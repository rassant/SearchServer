#include <iostream>
#include "read_input_functions.h"
#include <string>



std::string 
ReadLine ( ){
    std::string s;
    getline ( std::cin, s );
    return s;
}



int 
ReadLineWithNumber ( ){
    int result = 0;
    std::cin>> result;
    ReadLine ( );
    return result;
}