//
// Created by 杨文宇 on 2018/11/7.
//
#include <iostream>
#include <memory>
#include <fc/io/raw_fwd.hpp>
#include <fc/io/datastream.hpp>
#include <network/message_handlers.hpp>
#include <protocol/message.pb.h>
#include <protocol/message_factory.hpp>
#include <protocol/utility.hpp>
#include <database_controller/database_controller.hpp>
#include <database_controller/database_context.hpp>



using namespace action_message;


template<typename T>
static inline std::shared_ptr<T> get_data(int32_t key) {
    return std::shared_ptr<T>{dynamic_cast<T*>(message_factory::create(key)) };
}

inline static std::unique_ptr<database_context> get_context(rpc_request &req) {
    account_name receiver(req.receiver());
    return std::make_unique<database_context>(receiver,database_controller::get().get_state_db());
}

inline static void handle_exception(rpc_response &resp,fc::exception &e) {
    auto err = resp.mutable_err();
    err->set_code(e.code());
    err->set_msg(e.what());
}

void handler::send_response(rpc_response &resp, connection_ptr conn) {
    std::unique_ptr<serializer> serial{new serializer};
    serial->serialize(resp);
    conn->sync_write(serial->data(),serial->length());
}

void db_store_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_store_i64_response);
    std::cout << "db_store_i64_handler" << std::endl;
    try {
        auto data_ptr = get_data<db_store_i64>( req.msg_id() );
        auto cxt = get_context(req);
        int result = cxt->db_store_i64(data_ptr->scope(),data_ptr->table(),data_ptr->payer(),data_ptr->id(),data_ptr->buffer().c_str(),data_ptr->buffer_size());
        resp.set_result(result);
    } catch(fc::exception &e) {
        handle_exception(resp,e);
    }

}

void db_update_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {

    resp.set_msg_id(db_update_i64_response);
    std::cout << "db_update_i64_handler" << std::endl;
    try {
        auto data_ptr = get_data<db_update_i64>( req.msg_id() );
        auto cxt = get_context(req);
        cxt->db_update_i64(data_ptr->itr(),data_ptr->payer(),data_ptr->buffer().data(),data_ptr->buffer_size());
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
}

void db_remove_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_remove_i64_response);
    std::cout << "db_remove_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_remove_i64>( req.msg_id() );
        auto cxt = get_context(req);
        cxt->db_remove_i64(data_ptr->itr());
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }

}

void db_get_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_get_i64_response);
    std::cout << "db_get_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_get_i64>( req.msg_id() );
        auto cxt = get_context(req);
        std::string *buffer = resp.mutable_data();
        buffer->reserve(data_ptr->buffer_size());
        cxt->db_get_i64(data_ptr->itr(),buffer->data(),data_ptr->buffer_size());
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_next_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_next_i64_response);
    std::cout << "db_next_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_next_64>( req.msg_id() );
        auto cxt = get_context(req);
        uint64_t primary{};
        int result = cxt->db_next_i64(data_ptr->itr(),primary);
        resp.set_result(result);
        resp.set_result_primary(primary);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
}

void db_previous_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_previous_i64_response);
    std::cout << "db_previous_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_previous_i64>( req.msg_id() );
        auto cxt = get_context(req);
        uint64_t primary{};
        int result = cxt->db_previous_i64(data_ptr->itr(),primary);
        resp.set_result(result);
        resp.set_result_primary(primary);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_find_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_find_i64_response);
    std::cout << "db_find_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_find_i64>( req.msg_id() );
        auto cxt = get_context(req);
        int result = cxt->db_find_i64(data_ptr->code(),data_ptr->scope(),data_ptr->table(),data_ptr->id());
        resp.set_result(result);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_end_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_end_i64_response);
    std::cout << "db_end_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_end_i64>( req.msg_id() );
        auto cxt = get_context(req);
        int result = cxt->db_end_i64(data_ptr->code(),data_ptr->scope(),data_ptr->table());
        resp.set_result(result);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_lowerbound_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_lowerbound_i64_response);
    std::cout << "db_lowerbound_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_lowerbound_i64>( req.msg_id() );
        auto cxt = get_context(req);
        int result = cxt->db_lowerbound_i64(data_ptr->code(),data_ptr->scope(),data_ptr->table(),data_ptr->id());
        resp.set_result(result);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_upperbound_i64_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_upperbound_i64_response);
    std::cout << "db_upperbound_i64_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_upperbound_i64>( req.msg_id() );
        auto cxt = get_context(req);
        int result = cxt->db_upperbound_i64(data_ptr->code(),data_ptr->scope(),data_ptr->table(),data_ptr->id());
        resp.set_result(result);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_idx64_store_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_idx64_store_response);
    std::cout << "db_idx64_store_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_idx64_store>( req.msg_id() );
        auto cxt = get_context(req);
        int result = cxt->_idx64.store(data_ptr->scope(),data_ptr->table(),data_ptr->payer(),data_ptr->id(),data_ptr->secondary());
        resp.set_result(result);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_idx64_update_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_idx64_update_response);

    std::cout << "db_idx64_update_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_idx64_update>( req.msg_id() );
        auto cxt = get_context(req);
        cxt->_idx64.update(data_ptr->itr(),data_ptr->payer(),data_ptr->secondary());
    } catch(fc::exception &e) {
        handle_exception(resp,e);
    }
}

void db_idx64_remove_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_idx64_remove_response);
    std::cout << "db_idx64_remove_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_idx64_remove>( req.msg_id() );
        auto cxt = get_context(req);
        cxt->_idx64.remove(data_ptr->itr());
    } catch(fc::exception &e) {
        handle_exception(resp,e);
    }
}

void db_idx64_find_secondary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_idx64_find_secondary_response);

    std::cout << "db_idx64_find_secondary_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_idx64_find_secondary>( req.msg_id() );
        auto cxt = get_context(req);
        uint64_t primary{};
        int result = cxt->_idx64.find_secondary(data_ptr->code(),data_ptr->scope(),data_ptr->table(),data_ptr->secondary(),primary);
        resp.set_result(result);
        resp.set_result_primary(primary);
    } catch (fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_idx64_find_primary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    resp.set_msg_id(db_idx64_find_primary_response);
    std::cout << "db_idx64_find_primary_handler" << std::endl;

    try {
        auto data_ptr = get_data<db_idx64_find_primary>( req.msg_id() );
        auto cxt = get_context(req);
        uint64_t secondary{};
        int result = cxt->_idx64.find_primary(data_ptr->code(),data_ptr->scope(),data_ptr->table(),secondary,data_ptr->primary());
        resp.set_result(result);
        resp.set_result_secondary(secondary);
    } catch(fc::exception &e) {
        handle_exception(resp,e);
    }
    
}

void db_idx64_lowerbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx64_lowerbound>( req.msg_id() );
    std::cout << "db_idx64_lowerbound_handler" << std::endl;

    resp.set_msg_id(db_idx64_lowerbound_response);
    
}

void db_idx64_upperbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx64_upperbound>( req.msg_id() );
    std::cout << "db_idx64_upperbound_handler" << std::endl;

    resp.set_msg_id(db_idx64_upperbound_response);
    
}

void db_idx64_end_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx64_end>( req.msg_id() );
    std::cout << "db_idx64_end_handler" << std::endl;
}

void db_idx64_next_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx64_next>( req.msg_id() );
    std::cout << "db_idx64_next_handler" << std::endl;

    resp.set_msg_id(db_idx64_next_response);
    
}

void db_idx64_previous_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx64_previous>( req.msg_id() );
    if(data_ptr->ParseFromString(req.data()))
        std::cout << data_ptr->itr() << " " << data_ptr->primary() << std::endl;
    std::cout << "db_idx64_previous_handler" << std::endl;


    resp.set_msg_id(db_idx64_previous_response);
    
}

void db_idx128_store_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_store>( req.msg_id() );
    std::cout << "db_idx128_store_handler" << std::endl;

    resp.set_msg_id(db_idx128_store_response);
    
}

void db_idx128_update_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_update>( req.msg_id() );
    std::cout << "db_idx128_update_handler" << std::endl;

    resp.set_msg_id(db_idx128_update_response);
    
}

void db_idx128_remove_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_remove>( req.msg_id() );
    std::cout << "db_idx128_remove_handler" << std::endl;

    resp.set_msg_id(db_idx128_remove_response);
    
}

void db_idx128_find_secondary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_find_secondary>( req.msg_id() );
    std::cout << "db_idx128_find_secondary_handler" << std::endl;

    resp.set_msg_id(db_idx128_find_secondary_response);
    
}

void db_idx128_find_primary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_find_primary>( req.msg_id() );
    std::cout << "db_idx128_find_primary_handler" << std::endl;

    resp.set_msg_id(db_idx128_find_primary_response);
    
}

void db_idx128_lowerbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_lowerbound>( req.msg_id() );
    std::cout << "db_idx128_lowerbound_handler" << std::endl;

    resp.set_msg_id(db_idx128_lowerbound_response);
    
}

void db_idx128_upperbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_upperbound>( req.msg_id() );
    std::cout << "db_idx128_upperbound_handler" << std::endl;

    resp.set_msg_id(db_idx128_upperbound_response);
    
}

void db_idx128_end_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_end>( req.msg_id() );
    std::cout << "db_idx128_end_handler" << std::endl;

    resp.set_msg_id(db_idx128_end_response);
    
}

void db_idx128_next_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_next>( req.msg_id() );
    std::cout << "db_idx128_next_handler" << std::endl;
}

void db_idx128_previous_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx128_previous>( req.msg_id() );
    std::cout << "db_idx128_previous_handler" << std::endl;

    resp.set_msg_id(db_idx128_previous_response);
    
}

void db_idx256_store_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_store>( req.msg_id() );
    std::cout << "db_idx256_store_handler" << std::endl;

    resp.set_msg_id(db_idx256_store_response);
    
}

void db_idx256_update_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_update>( req.msg_id() );
    std::cout << "db_idx256_update_handler" << std::endl;

    resp.set_msg_id(db_idx256_update_response);
    
}

void db_idx256_remove_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_remove>( req.msg_id() );
    std::cout << "db_idx256_remove_handler" << std::endl;

    resp.set_msg_id(db_idx256_remove_response);
}

void db_idx256_find_secondary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_find_secondary>( req.msg_id() );
    std::cout << "db_idx256_find_secondary_handler" << std::endl;

    resp.set_msg_id(db_idx256_find_secondary_response);
}

void db_idx256_find_primary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_find_primary>( req.msg_id() );
    std::cout << "db_idx256_find_primary_handler" << std::endl;

    resp.set_msg_id(db_idx256_find_primary_response);
}

void db_idx256_lowerbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_lowerbound>( req.msg_id() );
    std::cout << "db_idx256_lowerbound_handler" << std::endl;
    resp.set_msg_id(db_idx256_lowerbound_response);
}

void db_idx256_upperbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_upperbound>( req.msg_id() );
    std::cout << "db_idx256_upperbound_handler" << std::endl;

    resp.set_msg_id(db_idx256_upperbound_response);
}

void db_idx256_end_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_end>( req.msg_id() );
    std::cout << "db_idx256_end_handler" << std::endl;

    resp.set_msg_id(db_idx256_end_response);
}

void db_idx256_next_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_next>( req.msg_id() );
    std::cout << "db_idx256_next_handler" << std::endl;

    resp.set_msg_id(db_idx256_next_response);
}

void db_idx256_previous_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx256_previous>( req.msg_id() );
    std::cout << "db_idx256_previous_handler" << std::endl;
    resp.set_msg_id(db_idx256_previous_response);
}

void db_idx_double_store_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_store>( req.msg_id() );
    std::cout << "db_idx_double_store_handler" << std::endl;

    resp.set_msg_id(db_idx_double_store_response);
}

void db_idx_double_update_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_update>( req.msg_id() );
    std::cout << "db_idx_double_update_handler" << std::endl;

    resp.set_msg_id(db_idx_double_update_response);
}

void db_idx_double_remove_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_remove>( req.msg_id() );
    std::cout << "db_idx_double_remove_handler" << std::endl;

    resp.set_msg_id(db_idx_double_remove_response);
}

void db_idx_double_find_secondary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_find_secondary>( req.msg_id() );
    std::cout << "db_idx_double_find_secondary_handler" << std::endl;

    resp.set_msg_id(db_idx_double_find_secondary_response);
}

void db_idx_double_find_primary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_find_primary>( req.msg_id() );
    std::cout << "db_idx_double_find_primary_handler" << std::endl;

    resp.set_msg_id(db_idx_double_find_primary_response);
}

void db_idx_double_lowerbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_lowerbound>( req.msg_id() );
    std::cout << "db_idx_double_lowerbound_handler" << std::endl;

    resp.set_msg_id(db_idx_double_lowerbound_response);
}

void db_idx_double_upperbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_upperbound>( req.msg_id() );
    std::cout << "db_idx_double_upperbound_handler" << std::endl;

    resp.set_msg_id(db_idx_double_upperbound_response);
}

void db_idx_double_end_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_end>( req.msg_id() );
    std::cout << "db_idx_double_end_handler" << std::endl;

    resp.set_msg_id(db_idx_double_end_response);
}

void db_idx_double_next_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_next>( req.msg_id() );
    std::cout << "db_idx_double_next_handler" << std::endl;

    resp.set_msg_id(db_idx_double_next_response);
}

void db_idx_double_previous_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_double_previous>( req.msg_id() );
    std::cout << "db_idx_double_previous_handler" << std::endl;

    resp.set_msg_id(db_idx_double_previous_response);
}

void db_idx_long_double_store_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_store>( req.msg_id() );
    std::cout << "db_idx_long_double_store_handler" << std::endl;

    resp.set_msg_id(db_idx_long_double_store_response);
}

void db_idx_long_double_update_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_update>( req.msg_id() );
    std::cout << "db_idx_long_double_update_handler" << std::endl;
    resp.set_msg_id(db_idx_long_double_update_response);
}

void db_idx_long_double_remove_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_remove>( req.msg_id() );
    std::cout << "db_idx_long_double_remove_handler" << std::endl;
    resp.set_msg_id(db_idx_long_double_remove_response);
}

void db_idx_long_double_find_secondary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_find_secondary>( req.msg_id() );
    std::cout << "db_idx_long_double_find_secondary_handler" << std::endl;

    resp.set_msg_id(db_idx_long_double_find_secondary_response);
}

void db_idx_long_double_find_primary_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_find_primary>( req.msg_id() );
    std::cout << "db_idx_long_double_find_primary_handler" << std::endl;
    resp.set_msg_id(db_idx_long_double_find_primary_response);
}

void db_idx_long_double_lowerbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_lowerbound>( req.msg_id() );
    std::cout << "db_idx_long_double_lowerbound_handler" << std::endl;

    resp.set_msg_id(db_idx_long_double_lowerbound_response);
}

void db_idx_long_double_upperbound_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_upperbound>( req.msg_id() );
    std::cout << "db_idx_long_double_upperbound_handler" << std::endl;

    resp.set_msg_id(db_idx_long_double_upperbound_response);
}

void db_idx_long_double_end_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_end>( req.msg_id() );
    std::cout << "db_idx_long_double_end_handler" << std::endl;

    resp.set_msg_id(db_idx_long_double_end_response);
}

void db_idx_long_double_next_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_next>( req.msg_id() );
    std::cout << "db_idx_long_double_next_handler" << std::endl;
    resp.set_msg_id(db_idx_long_double_next_response);
}

void db_idx_long_double_previous_handler::handle_message(rpc_request &req,rpc_response &resp) {
    
    auto data_ptr = get_data<db_idx_long_double_previous>( req.msg_id() );
    std::cout << "db_idx_long_double_previous_handler" << std::endl;
    resp.set_msg_id(db_idx_long_double_previous_response);
}

void check_transaction_authorization_handler::handle_message(rpc_request &req,rpc_response &resp) {
    auto data_ptr = get_data<check_transaction_authorization>(req.msg_id());
    std::cout << "check_transaction_authorization_handler" << std::endl;
    resp.set_msg_id(check_transaction_authorization_response);
}

void check_permission_authorization_handler::handle_message(rpc_request &req,rpc_response &resp) {
    auto data_ptr = get_data<check_permission_authorization>(req.msg_id());
    std::cout << "check_permission_authorization_handler" << std::endl;
    resp.set_msg_id(check_permission_authorization_response);
}

void get_permission_last_used_handler::handle_message(rpc_request &req,rpc_response &resp) {
    auto data_ptr  = get_data<get_permission_last_used>(req.msg_id());
    std::cout << "get_permission_last_used_handler" << std::endl;
    resp.set_msg_id(get_permission_last_used_response);
}

void get_account_creation_time_handler::handle_message(rpc_request &req,rpc_response &resp) {
    auto data_ptr = get_data<get_account_creation_time>(req.msg_id());
    std::cout << "get_account_creation_time_handler" << std::endl;
    resp.set_msg_id(get_account_creation_time_response);
}

void session_push_handler::handle_message(rpc_request &req, rpc_response &resp) {

    resp.set_msg_id(session_push_response);
}

void session_squash_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_squash_response);
}

void session_undo_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_undo_response);
}

void session_revision_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_revision_response);
}

void session_commit_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_commit_response);
}

void session_undo_all_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_undo_all_response);
}

void session_type_id_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_type_id_response);
}

void session_row_count_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_row_count_response);
}

void session_type_name_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_type_name_response);
}

void session_remove_object_handler::handle_message(rpc_request &req, rpc_response &resp) {
    resp.set_msg_id(session_remove_object_response);
}