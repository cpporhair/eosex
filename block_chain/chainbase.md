##
chainbase::database中关键结构：
   template<typename T>
   class oid {
      public:
         oid( int64_t i = 0 ):_id(i){}

         oid& operator++() { ++_id; return *this; }

         friend bool operator < ( const oid& a, const oid& b ) { return a._id < b._id; }
         friend bool operator > ( const oid& a, const oid& b ) { return a._id > b._id; }
         friend bool operator == ( const oid& a, const oid& b ) { return a._id == b._id; }
         friend bool operator != ( const oid& a, const oid& b ) { return a._id != b._id; }
         friend std::ostream& operator<<(std::ostream& s, const oid& id) {
            s << boost::core::demangle(typeid(oid<T>).name()) << '(' << id._id << ')'; return s;
         }

         int64_t _id = 0;
   };

   template<uint16_t TypeNumber, typename Derived>
   struct object
   {
      typedef oid<Derived> id_type;
      static const uint16_t type_id = TypeNumber;
   };
   者两个类是建立在database中的存储对象的关键，每个对象都是继承自object


##
controller::db中存储的数据结构：
    下面的每个数据结构都是继承自chainbase中的object.
    account_object，并建立索引：account_index,关键字段如下：
    struct account_object {
        id_type            id;        //类型id值
        account_name       name;      //账户名
        uint8_t            vm_type;
        uint8_t            vm_version;
        ...
        block_timestamp_type creation_date;
        ...
    };
    作用是存储账户信息

    account_sequence_object,并建立索引：account_sequence_index,关键字段如下：
    struct account_sequence_object {
        id_type      id;
        account_name name;
        uint64_t     recv_sequence = 0;
        uint64_t     auth_sequence = 0;
        uint64_t     code_sequence = 0;
        uint64_t     abi_sequence  = 0;
    };
    账户相关的序列号信息

    table_id_object,并建立索引：table_id_multi_index,关键字段如下：
    class table_id_object {
        id_type      id;
        account_name code;
        scope_name   scope;
        table_name   table;
        account_name payer;
        uint32_t     count = 0;
    };
    数据表索引

    key_value_object,并建立索引：key_value_index,关键字段如下：
    struct key_value_object {
        id_type      id;
        table_id     t_id;
        uint64_t     primary_key;
        account_name payer = 0;
        shared_string value;
    };

    index64_index,index128_index,index256_index,index_double_index,index_long_double_index均是二级索引定义于contract_table_objects.hpp:
    template<typename SecondaryKey, uint64_t ObjectTypeId, typename SecondaryKeyLess = std::less<SecondaryKey> >
    struct secondary_index
    {
        struct index_object : public chainbase::object<ObjectTypeId,index_object> {
            OBJECT_CTOR(index_object)
            typedef SecondaryKey secondary_key_type;

            typename chainbase::object<ObjectTypeId,index_object>::id_type       id;
            table_id      t_id;
            uint64_t      primary_key;
            account_name  payer = 0;
            SecondaryKey  secondary_key;
        };


        typedef chainbase::shared_multi_index_container<
            index_object,
            indexed_by<
            ordered_unique<tag<by_id>, member<index_object, typename index_object::id_type, &index_object::id>>,
            ordered_unique<tag<by_primary>,
                composite_key< index_object,
                    member<index_object, table_id, &index_object::t_id>,
                    member<index_object, uint64_t, &index_object::primary_key>
                >,
                composite_key_compare< std::less<table_id>, std::less<uint64_t> >
            >,
            ordered_unique<tag<by_secondary>,
                composite_key< index_object,
                    member<index_object, table_id, &index_object::t_id>,
                    member<index_object, SecondaryKey, &index_object::secondary_key>,
                    member<index_object, uint64_t, &index_object::primary_key>
                >,
                composite_key_compare< std::less<table_id>, SecondaryKeyLess, std::less<uint64_t> >
            >
            >
        > index_index;
    };

    global_property_object，并建立索引：global_property_multi_index,关键字段如下：
    struct global_property_object {
        id_type id;
        optional<block_num_type> proposed_schedule_block_num;
        shared_producer_schedule_type proposed_schedule;
        chain_config configuration;
    };

    class dynamic_global_property_object,并建立索引：dynamic_global_property_multi_index,关键字段如下:
    class dynamic_global_property_object {
        id_type _id;
        uint64_t global_action_sequence = 0;
    };

    block_summary_object,并建立索引：block_summary_multi_index,关键字段如下：
    class block_summary_object {
        id_type id;
        block_id_type block_id;
    }

    transaction_object,并建立索引：transaction_multi_index,关键字段如下：
    struct transaction_object {
        id_type id;
        time_point_sec expiration;
        transaction_id_type trx_id;
    }

    generated_transaction_object,并建立索引:generated_transaction_multi_index,关键字段如下：
    class generated_transaction_object {
        id_type id;
        transaction_id_type trx_id;
        account_name  sender;
        uint128_t  sender_id;
        account_name payer;
        time_point delay_until;
        time_point expiration;
        time_point published;
        shared_string packed_trx;
    }

    permission_object,并建立索引：permission_index,关键字段如下：
    class permission_object {
        id_type id;
        permission_usage_object::id_type usage_id;
        id_type parent;
        account_name owner;
        permission_name name;
        time_point last_updated;
        shared_authority auth;
    }

    permission_usage_object,并建立索引:permission_usage_index,关键字段如下：
    class permission_usage_object {
        id_type id;
        time_out last_used;
    }

    permission_link_object,并建立索引：permission_link_index,关键字段如下：
    class permission_link_object {
        id_type id;
        account_name account;
        account_name code;
        action_name message_type;
        permission_name required_permission;
    }

    resource_limit_object,并建立索引：resouce_limits_index,关键字段如下：
    struct resource_limit_object {
        id_type id;
        account_name owner;
        bool pending = false;
        int64_t net_weight = -1;
        int64_t cpu_weight = -1;
        int64_t ram_bytes = -1;
    }

    resource_usage_object,并建立索引：resource_usage_index,关键字段如下：
    struct resource_usage_object {
        id_type id;
        account_name owner;
        usage_accumulator net_usage;
        usage_accumulator cpu_usage;
        uint64_t ram_usage = 0;
    }

    resource_limits_state_object,并建立索引：resource_limits_state_index,关键字段如下：
    struct resource_limits_state_object {
        id_type id;
        usage_accumulator average_block_net_usage;
        usage_accumulator average_block_cpu_usage;
        int64_t pending_net_usage;
        int64_t pending_cpu_usage;
        int64_t total_net_weight;
        int64_t total_cpu_weight;
        int64_t total_ram_bytes;
    }

    以上结构均通过宏CHAINBASE_SET_INDEX_TYPE建立索引。