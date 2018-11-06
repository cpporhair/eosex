//
// Created by 杨文宇 on 2018/11/2.
//
#pragma once

#include <stdint.h>

namespace eosio {
    namespace chain {
        namespace config {
            typedef __uint128_t uint128_t;

            const static auto default_blocks_dir_name    = "blocks";
            const static auto reversible_blocks_dir_name = "reversible";
            const static auto default_reversible_cache_size = 340*1024*1024ll;/// 1MB * 340 blocks based on 21 producer BFT delay
            const static auto default_reversible_guard_size = 2*1024*1024ll;/// 1MB * 340 blocks based on 21 producer BFT delay

            const static auto default_state_dir_name     = "state";
            const static auto forkdb_filename            = "forkdb.dat";
            const static auto default_state_size            = 1*1024*1024*1024ll;
            const static auto default_state_guard_size      =    128*1024*1024ll;

            const static int      block_interval_ms = 500; //pre value 500
            const static int      block_interval_us = block_interval_ms*1000;
            const static uint64_t block_timestamp_epoch = 946684800000ll; // epoch is year 2000.
        }
    }
}