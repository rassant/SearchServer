#include <string_view>
#include <vector>

#include "../core/concurrent_map.h"
#include "../core/search_server.h"

void RunConcurrentUpdates ( ConcurrentMap < int, int >& cm, size_t thread_count, int key_count );
void TestConcurrentUpdate ( );
void TestReadAndWrite ( );
void TestSpeedup ( );
void StartSpeedTest ();

template <typename ExecutionPolicy>
void Test (  std::string_view mark
           , const SearchServer& search_server
           , const std::vector<std::string>& queries
           , ExecutionPolicy&& policy) 
{
  LOG_DURATION(mark);
  double total_relevance = 0;
  for (const std::string_view query : queries) {
    for (const auto& document : search_server.FindTopDocuments(policy, query)) {
      total_relevance += document.relevance;
    }
  }
  std::cerr <<"релевантность = " << total_relevance << std::endl;
}
#define TEST(policy) Test(#policy, search_server, queries, execution::policy)
