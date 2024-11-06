#pragma once

#include <algorithm> 
#include <cstdlib> 
#include <future> 
#include <map> 
#include <numeric> 
#include <random> 
#include <string> 
#include <vector> 

#include "../utils/log_duration.h"



template < typename Key, typename Value >
class ConcurrentMap {
    public:
        // структура Access { ссылка и mutex }
        struct Access {
            Value& ref_to_value;
            std::unique_lock < std::mutex > lock;

            Access ( Value& value, std::unique_lock < std::mutex > lock )
                : ref_to_value ( value ), lock ( std::move ( lock ))
            {
            }
        };
        // конструктор ConcurrentMap ( count )
        explicit ConcurrentMap ( size_t bucket_count )
            : buckets_ ( bucket_count ), mutexes_ ( bucket_count )
        {
        }
        Access operator[]( const Key& key )
        {
            size_t bucket_index = GetBucketIndex ( key );
            std::unique_lock < std::mutex > lock ( mutexes_[bucket_index] );
            auto& map = buckets_[bucket_index];

            auto it = map.find( key );
            if ( it != map.end( )) {
                return Access ( it-> second, std::move ( lock ));
            } else {
                it = map.emplace( key, Value ( )).first;
                return Access ( it-> second, std::move ( lock ));
            }
        }
        void erase ( Key key )
        {
            size_t bucket_index = GetBucketIndex ( key );
            std::unique_lock < std::mutex > lock ( mutexes_[bucket_index] );
            auto& map = buckets_[bucket_index];

            map.erase( key );
        }
        
        // создание map из векторов map ( собираем в корзину все map-ы ) BuildOrdinaryMap ( )
        std::map < Key, Value > BuildOrdinaryMap ( )
        {
            std::map < Key, Value > result;
            std::unique_lock < std::mutex > lock ( build_mutex_ );

            for ( size_t i = 0; i < buckets_.size( ); ++i ){
                std::unique_lock < std::mutex > bucket_lock ( mutexes_[i] );
                result.insert( buckets_[i].begin( ), buckets_[i].end( ));
            }
            return result;
        }

    private:
        // возвращает ключ из хэш таблицы GetBucketIndex
        size_t GetBucketIndex ( const Key& key )
        {
            return std::hash < Key >{}( key )% buckets_.size( );
        }
        std::vector < std::map < Key, Value >> buckets_;
        std::vector < std::mutex > mutexes_;
        std::mutex build_mutex_;
};
