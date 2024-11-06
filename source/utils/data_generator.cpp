#include "data_generator.h"
#include <algorithm> 

using namespace std;
string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    int u_length = max (0, length);
    word.reserve(static_cast<size_t>(u_length));
    for (int i = 0; i < u_length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}
vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(static_cast<size_t>(word_count));
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

std::string GenerateQuery(std::mt19937& generator, const std::vector<std::string>& dictionary, int word_count, double minus_prob) {
    std::string query;
    std::uniform_int_distribution<size_t> dist(0, dictionary.size() - 1); 
    std::uniform_real_distribution<> real_dist(0.0, 1.0);

    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (real_dist(generator) < minus_prob) {
            query.push_back('-');
        }
        if (!dictionary.empty()) { 
            query += dictionary[dist(generator)];
        }
    }
    return query;
}


vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    int u_query_count = max (0, query_count);
    queries.reserve(static_cast<size_t>(u_query_count));
    for (int i = 0; i < u_query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}

