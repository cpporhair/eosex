//
// Created by 杨文宇 on 2018/11/20.
//
#pragma once

#include <fc/utility.hpp>
#include <sstream>
#include <algorithm>
#include <set>
#include <eosio/chain/contract_table_objects.hpp>
#include <chainbase/chainbase.hpp>

using namespace eosio::chain;

class database_context {
private:
    template <typename T>
    class iterator_cache {
    public:
        iterator_cache() {
            _end_iterator_to_table.reserve(8);
            _iterator_to_object.reserve(32);
        }
        int cache_table(const table_id_object &obj) {
            auto itr = _table_cache.find(obj.id);
            if(itr != _table_cache.end())
                return itr->second.second;

            auto ei = index_to_end_iterator(_end_iterator_to_table.size());
            _end_iterator_to_table.push_back(&obj);
            _table_cache.emplace(obj.id,make_pair(&obj,ei));
            return ei;
        }
        const table_id_object& get_table(table_id_object::id_type id) const {
            auto itr = _table_cache.find(id);
            EOS_ASSERT(itr != _table_cache.end(),table_not_in_cache,"an invariant was broken,table should be in cache");
            return *itr->second.first;
        }
        int get_end_iterator_by_table_id(table_id_object::id_type id) const {
            auto itr = _table_cache.find(id);
            EOS_ASSERT(itr != _table_cache.end(),table_not_in_cache,"an invariant was broken,table should be in cache");
            return itr->second.second;
        }
        const table_id_object* find_table_by_end_iterator(int ei) const {
            EOS_ASSERT(ei < -1,invalid_table_iterator,"not and end iterator");
            auto idx = end_iterator_to_index(ei);
            if(idx >= _end_iterator_to_table.size()) return nullptr;
            return _end_iterator_to_table[idx];
        }
        const T& get(int iterator) {
            EOS_ASSERT( iterator != -1, invalid_table_iterator, "invalid iterator" );
            EOS_ASSERT( iterator >= 0, table_operation_not_permitted, "dereference of end iterator" );
            EOS_ASSERT( iterator < _iterator_to_object.size(), invalid_table_iterator, "iterator out of range" );
            auto result = _iterator_to_object[iterator];
            EOS_ASSERT(result,table_operation_not_permitted,"dereference of deleted object");
            return *result;
        }
        void remove(int iterator) {
            EOS_ASSERT( iterator != -1, invalid_table_iterator, "invalid iterator" );
            EOS_ASSERT( iterator >= 0, table_operation_not_permitted, "cannot call remove on end iterators" );
            EOS_ASSERT( iterator < _iterator_to_object.size(), invalid_table_iterator, "iterator out of range" );
            auto obj_ptr = _iterator_to_object[iterator];
            if( !obj_ptr ) return;
            _iterator_to_object[iterator] = nullptr;
            _object_to_iterator.erase( obj_ptr );
        }
        int add(const T& obj) {
            auto itr = _object_to_iterator.find(&obj);
            if(itr != _object_to_iterator.end())
                return itr->second;
            _iterator_to_object.push_back(&obj);
            _object_to_iterator[&obj] = _iterator_to_object.size() - 1;
            return _iterator_to_object.size() - 1;
        }
    private:
        map<table_id_object::id_type,pair<const table_id_object*,int>> _table_cache;
        vector<const table_id_object*>                                 _end_iterator_to_table;
        vector<const T*>                                               _iterator_to_object;
        map<const T*,int>                                              _object_to_iterator;
        inline size_t end_iterator_to_index(int ei) const { return (-ei - 2); }
        inline int index_to_end_iterator(size_t idx) const { return -(idx + 2); }
    };
    template<typename>
    struct array_size;

    template<typename T,size_t N>
    struct array_size<std::array<T,N>> {
        static constexpr size_t size = N;
    };

    template<typename SecondaryKey,typename SecondaryKeyProxy,typename SecondaryKeyProxyConst,typename Enable = void>
    class secondary_key_helper;

    template<typename SecondaryKey,typename SecondaryKeyProxy,typename SecondaryKeyProxyConst>
    class secondary_key_helper<SecondaryKey,SecondaryKeyProxy,SecondaryKeyProxyConst,
            typename std::enable_if<std::is_same<SecondaryKey,typename std::decay<SecondaryKeyProxy>::type>::value>::type> {
    public:
        typedef SecondaryKey secondary_key_type;
        static void set(secondary_key_type &sk_in_table,const secondary_key_type &sk_from_wasm) {
            sk_in_table = sk_from_wasm;
        }
        static void get(secondary_key_type &sk_from_wasm,const secondary_key_type &sk_in_table) {
            sk_from_wasm = sk_in_table;
        }
        static auto create_tuple(const table_id_object &obj,const secondary_key_type &secondary) {
            return boost::make_tuple(obj.id,secondary);
        }
    };

    template<typename SecondaryKey, typename SecondaryKeyProxy, typename SecondaryKeyProxyConst>
    class secondary_key_helper<SecondaryKey, SecondaryKeyProxy, SecondaryKeyProxyConst,
            typename std::enable_if<!std::is_same<SecondaryKey, typename std::decay<SecondaryKeyProxy>::type>::value &&
                                    std::is_pointer<typename std::decay<SecondaryKeyProxy>::type>::value>::type >
    {
    public:
        typedef SecondaryKey      secondary_key_type;
        typedef SecondaryKeyProxy secondary_key_proxy_type;
        typedef SecondaryKeyProxyConst secondary_key_proxy_const_type;

        static constexpr size_t N = array_size<SecondaryKey>::size;

        static void set(secondary_key_type& sk_in_table, secondary_key_proxy_const_type sk_from_wasm) {
            std::copy(sk_from_wasm, sk_from_wasm + N, sk_in_table.begin());
        }

        static void get(secondary_key_proxy_type sk_from_wasm, const secondary_key_type& sk_in_table) {
            std::copy(sk_in_table.begin(), sk_in_table.end(), sk_from_wasm);
        }

        static auto create_tuple(const table_id_object& tab, secondary_key_proxy_const_type sk_from_wasm) {
            secondary_key_type secondary;
            std::copy(sk_from_wasm, sk_from_wasm + N, secondary.begin());
            return boost::make_tuple( tab.id, secondary );
        }
    };

public:

    template<typename ObjectType,
             typename SecondaryKeyProxy = typename std::add_lvalue_reference<typename ObjectType::secondary_key_type>::type,
             typename SecondaryKeyProxyConst = typename std::add_lvalue_reference<typename std::add_const<typename ObjectType::secondary_key_type>::type>::type>
     class generic_index {
     public:
         typedef typename ObjectType::secondary_key_type      secondary_key_type;
         typedef SecondaryKeyProxy                            secondary_key_proxy_type;
         typedef SecondaryKeyProxyConst                       secondary_key_proxy_const_type;

         using secondary_key_helper_t = secondary_key_helper<secondary_key_type,secondary_key_proxy_type,secondary_key_proxy_const_type>;

         generic_index(database_context &c):_context(c){}
         int store(uint64_t scope,uint64_t table,const account_name &payer,uint64_t id,secondary_key_proxy_const_type value) {
             EOS_ASSERT(payer != account_name(), invalid_table_payer, "must specify a valid account to pay for new record");

             const auto &tb = _context.find_or_create_table(_context._receiver,scope,table,payer);

             const auto &obj = _context._db.create<ObjectType>([&](auto &o){
                 o.t_id = tb.id;
                 o.primary_key = id;
                 secondary_key_helper_t::set(o.secondary_key,value);
                 o.payer = payer;
             });

            _context._db.modify(tb,[&](auto& t) {
                ++t.count;
            });

            _context.update_db_usage(payer,config::billable_size_v<ObjectType>);
            _itr_cache.cache_table(tb);
            return _itr_cache.add(obj);
         }

         void remove(int iterator) {
             const auto &obj = _itr_cache.get(iterator);
             _context.update_db_usage(obj.payer,-(config::billable_size_v<ObjectType>));

             const auto &table_obj = _itr_cache.get_table(obj.t_id);
             EOS_ASSERT(table_obj.code == _context._receiver,table_access_violation,"db access violation");

             _context._db.modify(table_obj,[&](auto &t){
                 --t.count;
             });
             _context._db.remove(obj);

             if(table_obj.count == 0) {
                 _context.remove_table(table_obj);
             }

             _itr_cache.remove(iterator);
         }

         void update(int iterator,account_name payer,secondary_key_proxy_const_type secondary) {
             const auto &obj = _itr_cache.get(iterator);
             const auto &table_obj = _itr_cache.get_table(obj.t_id);
             EOS_ASSERT(table_obj.code == _context._receiver,table_access_violation,"db access violation");

             if(payer == account_name()) payer = obj.payer;

             int64_t billing_size = config::billable_size_v<ObjectType>;

             if(obj.payer != payer) {
                 _context.update_db_usage(obj.payer,-billing_size);
                 _context.update_db_usage(payer,billing_size);
             }

             _context._db.modify(obj,[&](auto &o){
                 secondary_key_helper_t::set(o.secondary_key,secondary);
                 o.payer = payer;
             });
         }

         int find_secondary(uint64_t code,uint64_t scope,uint64_t table,secondary_key_proxy_const_type secondary,uint64_t &primary) {
             auto tab = _context.find_table(code,scope,table);
             if(tab == nullptr) return -1;

             auto table_end_itr = _itr_cache.cache_table(*tab);
             const auto *obj = _context._db.find<ObjectType,by_secondary>(secondary_key_helper_t::create_tuple(*tab,secondary));
             if(obj == nullptr) return table_end_itr;

             primary = obj->primary_key;
             return _itr_cache.add(*obj);
         }

         int lowerbound_secondary(uint64_t code,uint64_t scope,uint64_t table,secondary_key_proxy_type secondary,uint64_t &primary) {
             auto tab = _context.find_table(code,scope,table);
             if(tab == nullptr) return -1;

             auto table_end_itr = _itr_cache.cache_table(*tab);

             const auto &idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type,by_secondary>();
             auto itr = idx.lower_bound(secondary_key_helper_t::create_tuple(*tab,secondary));
             if(itr == idx.end()) return table_end_itr;
             if(itr->t_id != tab->id) return table_end_itr;

             primary = itr->primary_key;
             secondary_key_helper_t::get(secondary,itr->secondary_key);

             return _itr_cache.add(*itr);
         }

         int upperbound_secondary(uint64_t code,uint64_t scope,uint64_t table,secondary_key_proxy_type secondary,uint64_t &primary) {
            auto tab = _context.find_table(code,scope,table);
            if(tab == nullptr) return -1;

            auto table_end_itr = _itr_cache.cache_table(*tab);
            
            const auto &idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type,by_secondary>();
            auto itr = idx.upper_bound(secondary_key_helper_t::create_tuple(*tab,secondary));
            if(itr == idx.end()) return table_end_itr;
            if(itr->t_id != tab->id) return table_end_itr;

            primary = itr->primary_key;
            secondary_key_helper_t::get(secondary,itr->secondary_key);
            return _itr_cache.add(*itr);
         }

         int end_secondary(uint64_t code,uint64_t scope,uint64_t table) {
             auto tab = _context.find_table(code,scope,table);
             if(tab == nullptr) return -1;

             return _itr_cache.cache_table(*tab);
         }

         int next_secondary(int iterator,uint64_t &primary) {
             if(iterator < -1) return -1;

             const auto &obj = _itr_cache.get(iterator);
             const auto &idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type,by_secondary>();

             auto itr = idx.iterator_to(obj);
             ++itr;
             if(itr == idx.end()() || itr->t_id != obj.t_id) return _itr_cache.get_end_iterator_by_table_id(obj.t_id);

             primary = itr->primary_key;
             return _itr_cache.add(*itr);
         }

         int previous_secondary(int iterator,uint64_t &primary) {
             const auto &idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type,by_secondary>();
             if(iterator < -1) {
                 auto tab = _itr_cache.find_table_by_end_iterator(iterator);
                 EOS_ASSERT(tab,invalid_table_iterator,"not a valid end iterator");

                 auto itr = idx.upper_bound(tab->id);
                 if(idx.begin() == idx.end() || itr == idx.begin()) return -1;
                 --itr;
                 if(itr->t_id != tab->id) return -1;
                 primary = itr->primary_key;
                 return _itr_cache.add(*itr);
             }

             const auto &obj = _itr_cache.get(iterator);
             auto itr = idx.iterator_to(obj);
             if(itr == idx.begin()) return -1;
             --itr;
             if(itr->t_id != obj.t_id) return -1;
             primary = itr->primary_key;
             return _itr_cache.add(*itr);
         }

         int find_primary(uint64_t code,uint64_t scope,uint64_t table,secondary_key_proxy_type secondary,uint64_t primary) {
            auto tab = _context.find_table(code,scope,table);
            if(tab == nullptr) return -1;

            auto table_end_itr = _itr_cache.cache_table(*tab);

            const auto *obj = _context._db.find<ObjectType,by_primary>(boost::make_tuple(tab->id,primary));
            if(obj == nullptr) return table_end_itr;
            secondary_key_helper_t::get(secondary,obj->secondary_key);
            return _itr_cache.add(*obj);
         }

         int lowerbound_primary(uint64_t code,uint64_t scope,uint64_t table,uint64_t primary) {
             auto tab = _context.find_table(code,scope,table);
             if(tab == nullptr) return -1;

             auto table_end_itr = _itr_cache.cache_table(*tab);
             
             const auto &idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type,by_primary>();
             auto itr = idx.lower_bound(boost::make_tuple(tab->id,primary));
             if(itr == idx.end()) return -1;
             if(itr->t_id != tab->id) return table_end_itr;

             return _itr_cache.add(*itr);
         }

         int upperbound_primary(uint64_t code,uint64_t scope,uint64_t table,uint64_t primary) {
             auto tab = _context.find_table( code, scope, table );
               if ( !tab ) return -1;

               auto table_end_itr = _itr_cache.cache_table( *tab );

               const auto& idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_primary>();
               auto itr = idx.upper_bound(boost::make_tuple(tab->id, primary));
               if (itr == idx.end()) return table_end_itr;
               if (itr->t_id != tab->id) return table_end_itr;

               _itr_cache.cache_table(*tab);
               return _itr_cache.add(*itr);
         }

         int next_primary(int iterator,uint64_t &primary) {
             if( iterator < -1 ) return -1; // cannot increment past end iterator of table

               const auto& obj = _itr_cache.get(iterator); // Check for iterator != -1 happens in this call
               const auto& idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_primary>();

               auto itr = idx.iterator_to(obj);
               ++itr;

               if( itr == idx.end() || itr->t_id != obj.t_id ) return _itr_cache.get_end_iterator_by_table_id(obj.t_id);

               primary = itr->primary_key;
               return _itr_cache.add(*itr);
         }

         int previous_primary(int iterator,uint64_t &primary) {
             const auto& idx = _context._db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_primary>();

               if( iterator < -1 ) // is end iterator
               {
                  auto tab = _itr_cache.find_table_by_end_iterator(iterator);
                  EOS_ASSERT( tab, invalid_table_iterator, "not a valid end iterator" );

                  auto itr = idx.upper_bound(tab->id);
                  if( idx.begin() == idx.end() || itr == idx.begin() ) return -1; // Empty table

                  --itr;

                  if( itr->t_id != tab->id ) return -1; // Empty table

                  primary = itr->primary_key;
                  return _itr_cache.add(*itr);
               }

               const auto& obj = _itr_cache.get(iterator); // Check for iterator != -1 happens in this call

               auto itr = idx.iterator_to(obj);
               if( itr == idx.begin() ) return -1; // cannot decrement past beginning iterator of table

               --itr;

               if( itr->t_id != obj.t_id ) return -1; // cannot decrement past beginning iterator of index

               primary = itr->primary_key;
               return _itr_cache.add(*itr);
         }

         void get(int iterator,uint64_t &primary,secondary_key_proxy_type secondary) {
            const auto& obj = _itr_cache.get( iterator );
            primary   = obj.primary_key;
            secondary_key_helper_t::get(secondary, obj.secondary_key);
         }

     private:
         database_context              &_context;
         iterator_cache<ObjectType>    _itr_cache;
     };

public:
    database_context(account_name &receiver,chainbase::database &db):
        _receiver(receiver),
        _db(db),
        _idx64(*this),
        _idx128(*this),
        _idx256(*this),
        _idx_double(*this),
        _idx_long_double(*this){}

public:
    generic_index<index64_object>                 _idx64;
    generic_index<index128_object>                _idx128;
    generic_index<index256_object,uint128_t*,const uint128_t*> _idx256;
    generic_index<index_double_object>            _idx_double;
    generic_index<index_long_double_object>       _idx_long_double;
    
public:
    void require_authorization(const account_name &account);
    bool has_authorization(const account_name &account) const;
    void require_authorization(const account_name &account,const permission_name &permission);


    void update_db_usage(const account_name &payer,int64_t delta);
    int db_store_i64(uint64_t scope,uint64_t table,const account_name &payer,uint64_t id,const char *buffer,size_t buffer_size);
    void db_update_i64(int iterator,account_name payer,const char *buffer,size_t buffer_size);
    void db_remove_i64(int iterator);
    int db_get_i64(int iterator,char *buffer,size_t buffer_size);
    int db_next_i64(int iterator,uint64_t &primary);
    int  db_previous_i64( int iterator, uint64_t& primary );
    int  db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
    int  db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
    int  db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
    int  db_end_i64( uint64_t code, uint64_t scope, uint64_t table );

private:
    const table_id_object* find_table(name code,name scope,name table);
    const table_id_object& find_or_create_table(name code,name scope,name table,const account_name payer);
    void  remove_table(const table_id_object &obj);

    int  db_store_i64( uint64_t code, uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size );

private:
    account_name                     _receiver;
    chainbase::database              &_db;
    iterator_cache<key_value_object> _key_value_cache;
};