#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <cstdint>

#include <chain/name.hpp>
#include <fc/crypto/ripemd160.hpp>

namespace eosio {
    namespace chain {

        using public_key_type  = fc::crypto::public_key;
        using private_key_type = fc::crypto::private_key;
        using signature_type   = fc::crypto::signature;

        using action_name      = name;
        using scope_name       = name;
        using account_name     = name;
        using permission_name  = name;
        using table_name       = name;

        using  std::vector;
        using  std::map;


        enum object_type
        {
            null_object_type = 0,
            account_object_type,
            account_sequence_object_type,
            permission_object_type,
            permission_usage_object_type,
            permission_link_object_type,
            UNUSED_action_code_object_type,
            key_value_object_type,
            index64_object_type,
            index128_object_type,
            index256_object_type,
            index_double_object_type,
            index_long_double_object_type,
            global_property_object_type,
            dynamic_global_property_object_type,
            block_summary_object_type,
            transaction_object_type,
            generated_transaction_object_type,
            producer_object_type,
            UNUSED_chain_property_object_type,
            account_control_history_object_type,     ///< Defined by history_plugin
            UNUSED_account_transaction_history_object_type,
            UNUSED_transaction_history_object_type,
            public_key_history_object_type,          ///< Defined by history_plugin
            UNUSED_balance_object_type,
            UNUSED_staked_balance_object_type,
            UNUSED_producer_votes_object_type,
            UNUSED_producer_schedule_object_type,
            UNUSED_proxy_vote_object_type,
            UNUSED_scope_sequence_object_type,
            table_id_object_type,
            resource_limits_object_type,
            resource_usage_object_type,
            resource_limits_state_object_type,
            resource_limits_config_object_type,
            account_history_object_type,              ///< Defined by history_plugin
            action_history_object_type,               ///< Defined by history_plugin
            reversible_block_object_type,
            OBJECT_TYPE_COUNT ///< Sentry value which contains the number of different object types
        };

        class account_object;
        class producer_object;

        using block_id_type       = fc::sha256;
        using checksum_type       = fc::sha256;
        using checksum256_type    = fc::sha256;
        using checksum512_type    = fc::sha512;
        using checksum160_type    = fc::ripemd160;
        using transaction_id_type = checksum_type;
        using digest_type         = checksum_type;
        using weight_type         = uint16_t;
        using block_num_type      = uint32_t;
        using share_type          = int64_t;
        using int128_t            = __int128;
        using uint128_t           = unsigned __int128;
        using bytes               = vector<char>;
    }
}