
#include "../core/concurrent_map.h"
#include "test_speed.h"
#include "test_example_functions.h"
#include "test_framework.h"
#include "../utils/data_generator.h"

using namespace std;

void TestSpeedup ( ){
    {
        ConcurrentMap < int, int > single_lock ( 1 );

        LOG_DURATION ( "Single lock" );
        RunConcurrentUpdates ( single_lock, 4, 50000 );
    }
    {
        ConcurrentMap < int, int > many_locks ( 100 );

        LOG_DURATION ( "100 locks" );
        RunConcurrentUpdates ( many_locks, 4, 50000 );
    }
}


void TestReadAndWrite ( ){
    ConcurrentMap < size_t, string > cm ( 5 );

    auto updater = [&cm] {
        for ( size_t i = 0; i < 50000; ++i ){
            cm[i].ref_to_value.push_back( 'a' );
        }
    };
    auto reader = [&cm] {
        vector < string > result ( 50000 );
        for ( size_t i = 0; i < result.size( ); ++i ){
            result[i] = cm[i].ref_to_value;
        }
        return result;
    };

    auto u1 = async (  updater);
    auto r1 = async (reader   );
    auto u2 = async (  updater);
    auto r2 = async (reader   );

    u1.get( );
    u2.get( );

    for ( auto f : { &r1, &r2} ){
        auto result = f-> get ( );
        ASSERT ( all_of ( result.begin( ), result.end( ), []( const string& s ){
            return s.empty( )|| s == "a" || s == "aa";
            /*return !s.empty() && s.find_first_not_of('a') == string::npos;*/
        } ));
    }
}



void TestConcurrentUpdate ( ){
    constexpr size_t THREAD_COUNT = 3;
    constexpr size_t KEY_COUNT = 50000;

    ConcurrentMap < int, int > cm ( THREAD_COUNT );
    RunConcurrentUpdates ( cm, THREAD_COUNT, KEY_COUNT );

    const auto result = cm.BuildOrdinaryMap( );
    ASSERT_EQUAL ( result.size( ), KEY_COUNT );
    for ( auto& [k, v] : result ){
        AssertEqual ( v, 6, "Key = " + to_string ( k ));
    }
}



void RunConcurrentUpdates ( ConcurrentMap < int, int >& cm, size_t thread_count, int key_count ){
    auto kernel = [&cm, key_count]( int seed ){
        int key = std::max(0, key_count); 
        vector < int > updates ( static_cast<size_t>(key) );

        iota ( begin ( updates ), end ( updates ), -key / 2 );
        int u_seed = std::max(0, seed); 
        shuffle ( begin ( updates ), end ( updates ), mt19937 ( static_cast<size_t>(u_seed) ));

        for ( int i = 0; i < 2; ++i ){
            for ( auto key : updates ){
                ++cm[key].ref_to_value;
            }
        }
    };

    vector < future < void >> futures;
    for ( size_t i = 0; i < thread_count; ++i ){
        futures.push_back( async ( kernel, i ));
    }
}



void StartSpeedTest ()
{
    TestRunner tr;
    RUN_TEST( tr, TestConcurrentUpdate );
    RUN_TEST( tr, TestReadAndWrite );
    RUN_TEST( tr, TestSpeedup );


    mt19937 generator;
    const auto dictionary = GenerateDictionary(generator, 10000, 10);
    const auto documents  = GenerateQueries(generator, dictionary, 10'000, 70);
    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
      search_server.AddDocument(static_cast<int>(i), documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }
    const auto queries = GenerateQueries(generator, dictionary, 100, 70);

    cerr << endl << "последовательное вычисление: ";
    TEST(seq);
    cerr << "параллейное вычисление: ";
    TEST(par);

}
