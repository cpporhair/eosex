//
// Created by 杨文宇 on 2018/11/20.
//
#include <database_controller/database_context.hpp>

void database_context::update_db_usage(const account_name &payer, int64_t delta) {

}

int database_context::db_store_i64(uint64_t scope, uint64_t table, const account_name &payer, uint64_t id,
                                   const char *buffer, size_t buffer_size) {
    return db_store_i64(_receiver,scope,table,payer,id,buffer,buffer_size);
}

int database_context::db_store_i64(uint64_t code, uint64_t scope, uint64_t table, const account_name &payer,
                                   uint64_t id, const char *buffer, size_t buffer_size) {
    const auto &tab = find_or_create_table(code,scope,table,payer);
    auto table_id = tab.id;

    EOS_ASSERT(payer != account_name(),invalid_table_payer,"must specify a valid account to pay for new record");

    const auto &obj = _db.create<key_value_object>([&](auto &o){
        o.t_id = table_id;
        o.primary_key = id;
        o.value.resize(buffer_size);
        o.payer = payer;
        memcpy(o.value.data(),buffer,buffer_size);
    });

    _db.modify(tab,[&](auto &t){
        ++t.count;
    });

    int64_t billable_size = (int64_t)(buffer_size + config::billable_size_v<key_value_object>);
    update_db_usage(payer,billable_size);

    _key_value_cache.cache_table(tab);
    return _key_value_cache.add(obj);
}

void database_context::db_update_i64(int iterator, account_name payer, const char *buffer, size_t buffer_size) {
    const key_value_object &obj = _key_value_cache.get(iterator);

    const auto &table_obj = _key_value_cache.get_table(obj.t_id);
    EOS_ASSERT(table_obj.code == _receiver,table_access_violation,"db access violation");

    const int64_t overhead = config::billable_size_v<key_value_object>;
    int64_t old_size = (int64_t)(obj.value.size() + overhead);
    int64_t new_size = (int64_t)(buffer_size + overhead);

    if(payer == account_name()) payer = obj.payer;

    if(account_name(obj.payer) != payer) {
        update_db_usage(obj.payer,-old_size);
        update_db_usage(payer,new_size);
    } else if(old_size != new_size) {
        update_db_usage(obj.payer,new_size - old_size);
    }

    _db.modify(obj,[&](auto &o){
        o.value.resize(buffer_size);
        memcpy(o.value.data(),buffer,buffer_size);
        o.payer = payer;
    });
}

void database_context::db_remove_i64(int iterator) {
    const key_value_object& obj = _key_value_cache.get( iterator );

    const auto& table_obj = _key_value_cache.get_table( obj.t_id );
    EOS_ASSERT( table_obj.code == _receiver, table_access_violation, "db access violation" );

//   require_write_lock( table_obj.scope );

    update_db_usage( obj.payer,  -(obj.value.size() + config::billable_size_v<key_value_object>) );

    _db.modify( table_obj, [&]( auto& t ) {
        --t.count;
    });
    _db.remove( obj );

    if (table_obj.count == 0) {
        remove_table(table_obj);
    }

    _key_value_cache.remove( iterator );
}

int database_context::db_get_i64(int iterator, char *buffer, size_t buffer_size) {
    const key_value_object& obj = _key_value_cache.get( iterator );

    auto s = obj.value.size();
    if( buffer_size == 0 ) return s;

    auto copy_size = std::min( buffer_size, s );
    memcpy( buffer, obj.value.data(), copy_size );

    return copy_size;
}

int database_context::db_next_i64(int iterator, uint64_t &primary) {
    if( iterator < -1 ) return -1; // cannot increment past end iterator of table

    const auto& obj = _key_value_cache.get( iterator ); // Check for iterator != -1 happens in this call
    const auto& idx = _db.get_index<key_value_index, by_scope_primary>();

    auto itr = idx.iterator_to( obj );
    ++itr;

    if( itr == idx.end() || itr->t_id != obj.t_id ) return _key_value_cache.get_end_iterator_by_table_id(obj.t_id);

    primary = itr->primary_key;
    return _key_value_cache.add( *itr );
}

int database_context::db_previous_i64(int iterator, uint64_t &primary) {
    const auto& idx = _db.get_index<key_value_index, by_scope_primary>();

    if( iterator < -1 ) // is end iterator
    {
        auto tab = _key_value_cache.find_table_by_end_iterator(iterator);
        EOS_ASSERT( tab, invalid_table_iterator, "not a valid end iterator" );

        auto itr = idx.upper_bound(tab->id);
        if( idx.begin() == idx.end() || itr == idx.begin() ) return -1; // Empty table

        --itr;

        if( itr->t_id != tab->id ) return -1; // Empty table

        primary = itr->primary_key;
        return _key_value_cache.add(*itr);
    }

    const auto& obj = _key_value_cache.get(iterator); // Check for iterator != -1 happens in this call

    auto itr = idx.iterator_to(obj);
    if( itr == idx.begin() ) return -1; // cannot decrement past beginning iterator of table

    --itr;

    if( itr->t_id != obj.t_id ) return -1; // cannot decrement past beginning iterator of table

    primary = itr->primary_key;
    return _key_value_cache.add(*itr);
}

int database_context::db_find_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
    const auto* tab = find_table( code, scope, table );
    if( !tab ) return -1;

    auto table_end_itr = _key_value_cache.cache_table( *tab );

    const key_value_object* obj = _db.find<key_value_object, by_scope_primary>( boost::make_tuple( tab->id, id ) );
    if( !obj ) return table_end_itr;

    return _key_value_cache.add( *obj );
}

int database_context::db_lowerbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
    const auto* tab = find_table( code, scope, table );
    if( !tab ) return -1;

    auto table_end_itr = _key_value_cache.cache_table( *tab );

    const auto& idx = _db.get_index<key_value_index, by_scope_primary>();
    auto itr = idx.lower_bound( boost::make_tuple( tab->id, id ) );
    if( itr == idx.end() ) return table_end_itr;
    if( itr->t_id != tab->id ) return table_end_itr;

    return _key_value_cache.add( *itr );
}

int database_context::db_upperbound_i64(uint64_t code, uint64_t scope, uint64_t table, uint64_t id) {
    const auto* tab = find_table( code, scope, table );
    if( !tab ) return -1;

    auto table_end_itr = _key_value_cache.cache_table( *tab );

    const auto& idx = _db.get_index<key_value_index, by_scope_primary>();
    auto itr = idx.upper_bound( boost::make_tuple( tab->id, id ) );
    if( itr == idx.end() ) return table_end_itr;
    if( itr->t_id != tab->id ) return table_end_itr;

    return _key_value_cache.add( *itr );
}

int database_context::db_end_i64(uint64_t code, uint64_t scope, uint64_t table) {
    const auto* tab = find_table( code, scope, table );
    if( !tab ) return -1;

    return _key_value_cache.cache_table( *tab );
}

const table_id_object* database_context::find_table(name code, name scope, name table) {
    return _db.find<table_id_object,by_code_scope_table>(boost::make_tuple(code,scope,table));
}

const table_id_object& database_context::find_or_create_table(name code, name scope, name table,
                                                              const account_name payer) {
    const auto* existing_tid =  _db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
    if (existing_tid != nullptr) {
        return *existing_tid;
    }

    update_db_usage(payer, config::billable_size_v<table_id_object>);

    return _db.create<table_id_object>([&](table_id_object &t_id){
        t_id.code = code;
        t_id.scope = scope;
        t_id.table = table;
        t_id.payer = payer;
    });
}

void database_context::remove_table(const table_id_object &obj) {
    update_db_usage(obj.payer, - config::billable_size_v<table_id_object>);
    _db.remove(obj);
}

void database_context::require_authorization(const account_name &account) {

}

void database_context::require_authorization(const account_name &account, const permission_name &permission) {

}

bool database_context::has_authorization(const account_name &account) const {

}
