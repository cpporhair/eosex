# EOS网络通信分析

eos网络通信主要分为两部分，一是p2p通信，即各个nodeos节点之间的通信，二是客户端与服务器之间REST API的通信，即cleos与nodeos的通信，EOS采用插件机制，前者主要在net_plugin中实现，后者主要在http_plugin中实现。

## p2p网络

### plugin初始化

nodeos启动的时候，会注册并初始化相应插件，需要注册的插件可以通过config.ini配置文件配置，也可以通过启动参数配置，nodeos main函数中通过initialize<...>()，调用各个插件的plugin_initialize()方法，实现插件的初始化，插件初始化：
```
if(!app().initialize<chain_plugin, http_plugin, net_plugin, producer_plugin>(argc, argv))
         return INITIALIZE_FAIL;
```
其中，网络部分主要看net_plugin和http_plugin，先分析net_plugin中实现的功能。
在net_plugin的plugin_initialize()中，首先会读取配置的各个参数，比如sync-fetch-span(一次同步区块的数量)，p2p-listen-endpoint(p2p本地监听端点)，p2p-peer-address(要连接的其他nodeos地址)等。然后通过chain_plugin获取chain_id，chain_id在p2p通信的时候会用到，判断是不是同链之间的通信。
```
	my->chain_plug = app().find_plugin<chain_plugin>();
    my->chain_id = app().get_plugin<chain_plugin>().get_chain_id();
```

### plugin启动

nodeos mian函数中，初始化插件后，会调用app().startup()启动各个已注册的插件，分别调用每个插件的plugin_startup()完成插件启动
```
void application::startup() {
   try {
      for (auto plugin : initialized_plugins)
         plugin->startup();
   } catch(...) {
      shutdown();
      throw;
   }
}
class plugin : public abstract_plugin {
	...
	virtual void startup() override {
        if(_state == initialized) {
           _state = started;
           static_cast<Impl*>(this)->plugin_requires([&](auto& plug){ plug.startup(); });
           static_cast<Impl*>(this)->plugin_startup();
           app().plugin_started(*this);
        }
        assert(_state == started); // if initial state was not initialized, final state cannot be started
     }
}
```
net_plugin在plugin_startup()中会做几项工作，先整体说一下，后面再具体分析：
* 1 监听本地p2p端口，接受连接并等待读取消息
```
void net_plugin::plugin_startup() {
	if( my->acceptor ) {
        my->acceptor->open(my->listen_endpoint.protocol());
        my->acceptor->set_option(tcp::acceptor::reuse_address(true));
        my->acceptor->bind(my->listen_endpoint);
        my->acceptor->listen();
        ilog("starting listener, max clients is ${mc}",("mc",my->max_client_count));
        my->start_listen_loop();
    }
    ...
}
void net_plugin_impl::start_listen_loop( ) {
	acceptor->async_accept( *socket, [socket,this]( boost::system::error_code ec ) {
    	...
    	connection_ptr c = std::make_shared<connection>( socket );
        connections.insert( c );
        start_session( c );
    });
}
```
* 2 绑定区块/交易广播信号槽
```
{
    chain::controller&cc = my->chain_plug->chain();
    cc.accepted_block_header.connect( boost::bind(&net_plugin_impl::accepted_block_header, my.get(), _1));
    cc.accepted_block.connect(  boost::bind(&net_plugin_impl::accepted_block, my.get(), _1));
    cc.irreversible_block.connect( boost::bind(&net_plugin_impl::irreversible_block, my.get(), _1));
    cc.accepted_transaction.connect( boost::bind(&net_plugin_impl::accepted_transaction, my.get(), _1));
    cc.applied_transaction.connect( boost::bind(&net_plugin_impl::applied_transaction, my.get(), _1));
    cc.accepted_confirmation.connect( boost::bind(&net_plugin_impl::accepted_confirmation, my.get(), _1));
}
my->incoming_transaction_ack_subscription = app().get_channel<channels::transaction_ack>().subscribe(boost::bind(&net_plugin_impl::transaction_ack, my.get(), _1));
```
* 3 定时监控nodeos connections连接和定时清理本地过期交易
```
my->start_monitors();
void net_plugin_impl::start_monitors() {
    connector_check.reset(new boost::asio::steady_timer( app().get_io_service()));
    transaction_check.reset(new boost::asio::steady_timer( app().get_io_service()));
    start_conn_timer();
    start_txn_timer();
}
```
* 4 主动连接本地配置的其他p2p-addr。
```
for( auto seed_node : my->supplied_peers ) {
	connect( seed_node );
}
```
---


### 节点间通信

下面分析下两个nodeos节点间通信流程(A的p2p-addr为192.168.1.101:9876，B的p2p-addr为192.168.1.102:9876)

#### p2p连接

在前面已经介绍了net_plugin启动时的几项工作，其中第4条主动连接其他p2p-addr节点，从这里开始分析。
假设节点B已经启动，已监听本地p2p端口；节点A在config.ini中配置了节点B的p2p-addr，即A的config.ini部分配置如下:
```
p2p-listen-endpoint = 0.0.0.0:9876
p2p-peer-address = 192.168.1.102:9876
```
A在启动时会调用net_plugin::connect()方法主动连接B，首先判断是否已经连接，如果没有连接会把host("192.168.1.102:9876")作为参数赋值给connection的成员变量peer_addr 构造connection_ptr，然后调用net_plugin_impl::connect()方法，在这里会去解析peer-addr，然后调用boost的asio库方法连接B，如果连接成功，再调用net_plugin_impl::start_session()--> net_plugin_impl::start_read_message()，开始异步读取消息。
```
string net_plugin::connect( const string& host ) {
    if( my->find_connection( host ) )
     return "already connected";

    connection_ptr c = std::make_shared<connection>(host);
    my->connections.insert( c );
    my->connect( c );
    return "added connection";
}
```
```
void net_plugin_impl::connect( connection_ptr c, tcp::resolver::iterator endpoint_itr ) {
	c->socket->async_connect( current_endpoint, [weak_conn, endpoint_itr, this] ( const boost::system::error_code& err ) {
        if( !err && c->socket->is_open() ) {
           if (start_session( c )) {
              c->send_handshake ();
           }
        }
        ...
    });
}
```
```
bool net_plugin_impl::start_session( connection_ptr con ) {
	...
    start_read_message( con );
}
```
```
void net_plugin_impl::start_read_message( connection_ptr conn ) {
	...
    boost::asio::async_read(...);
}
```
我们看到，如果A连接成功B开始一个session后，会接着调用c->send_handshake()给B发送一个handshake信息过去，这里就开始了A与B的p2p通信。

#### p2p消息发送与接收

	两个nodeos之间，是通过发送几种不同类型的message来实现信息交换，同步区块/交易，以及断连原因等，这些消息类型有handshake_message，go_away_message，time_message，notice_message，request_message，sync_request_message，signed_block，packed_transaction等。
    
##### handshake_message
	下面从发送接收handshake_message握手信息来入手分析p2p各种消息的通信流程。
	当A成功连接B后，会调用send_handshake()，在该方法中会生成一个handshake_message，然后会将该message放入connection的消息队列中，再通过boost::asio::async_write()发送给B。handshake_message中带有A的本地信息，包括chain_id(识别链)，node_id(识别peers，防止自己连自己)，公钥/签名，本地区块头号，区块头ID，不可逆区块号，不可逆区块ID等信息
```
struct handshake_message {
    uint16_t                   network_version = 0;
    chain_id_type              chain_id;
    fc::sha256                 node_id;
    chain::public_key_type     key;
    tstamp                     time;
    fc::sha256                 token;
    chain::signature_type      sig;
    string                     p2p_address;
    uint32_t                   last_irreversible_block_num = 0;
    block_id_type              last_irreversible_block_id;
    uint32_t                   head_num = 0;
    block_id_type              head_id;
    string                     os;
    string                     agent;
    int16_t                    generation;
};
```
前面我们假设了B已经启动，当A成功连接到B，并给B发送handshake_message后，B通过boost::asio::async_read()，读取到A给B发送的消息，然后处理解析消息，通过msgHandler调用net_plugin_impl::handle_message(c, msg)处理不同类型的消息，在net_plugin_impl中重载了handle_message()方法，当A发给B的是handshake_message，会调用net_plugin_impl::handle_message( connection_ptr c, const handshake_message &msg) 方法。在该方法中会判断handshake_message的有效性，比较chain_id，node_id，network_version以及判断是否分叉等，如果是正确有效的handshake_message，则会调用sync_master->recv_handshake(c,msg)，比较两个节点的区块信息，判断是否需要同步区块等。
```
void sync_manager::recv_handshake (connection_ptr c, const handshake_message &msg) {
	//获取本地(B)和peer(A)两节点的不可逆号
	controller& cc = chain_plug->chain();
    uint32_t lib_num = cc.last_irreversible_block_num( );
    uint32_t peer_lib = msg.last_irreversible_block_num;
    //获取B区块头号和头ID
    uint32_t head = cc.head_block_num( );
    block_id_type head_id = cc.head_block_id();
    //下面是比较两个节点信息，然后决定B节点给A节点发送什么类型的message
    if (head_id == msg.head_id) {
    	//头块ID相同，发送一个known_trx.mode = catch_up的notice_message，带着本地缓存交易的数目
        notice_message note;...
        return;
    }
    //如果B的头号小于A的不可逆号，B给A发送一个同步区块的请求信息
    if (head < peer_lib) {
    	start_sync( c, peer_lib);
        return;
    }
    //如果本地不可逆号大于消息的头号，则B给A发一个notice_message消息，告诉A此时B的不可逆号，让A来向B请求同步
    if (lib_num > msg.head_num ) {
    	if (msg.generation > 1 || c->protocol_version > proto_base) {
            notice_message note;
            note.known_trx.pending = lib_num;
            note.known_trx.mode = last_irr_catch_up;
            note.known_blocks.mode = last_irr_catch_up;
            note.known_blocks.pending = head;
            c->enqueue( note );
         }
         c->syncing = true;
         return;
    }
    //如果本地头号小于等于消息的头号，则B给A发送一个request_message，mode为 catch_up消息，请求A不可逆号与头号之间的区块，否则发送一个notice_message catch_up消息，带着本地头ID
    if (head <= msg.head_num ) {
        verify_catchup (c, msg.head_num, msg.head_id);
        return;
    } else {
    	if (msg.generation > 1 ||  c->protocol_version > proto_base) {
            notice_message note;
            note.known_trx.mode = none;
            note.known_blocks.mode = catch_up;
            note.known_blocks.pending = head;
            note.known_blocks.ids.push_back(head_id);
            c->enqueue( note );
         }
         c->syncing = true;
         return;
    }
}
```
同样的，B给A发送消息，A接受消息，采用相同的流程，B通过把message放入消息队列然后发送给A，A收到信息后，判断message type，调用不同的handle_message()方法，处理消息，后面会对每个消息类型的处理作具体分析。

##### request_message

request_message处理，先贴源码，从源码分析
```
void net_plugin_impl::handle_message( connection_ptr c, const request_message &msg) {
      switch (msg.req_blocks.mode) {
      case catch_up :
         peer_ilog(c,  "received request_message:catch_up");
         c->blk_send_branch( );
         break;
      case normal :
         peer_ilog(c, "received request_message:normal");
         c->blk_send(msg.req_blocks.ids);
         break;
      default:;
      }

      switch (msg.req_trx.mode) {
      case catch_up :
         c->txn_send_pending(msg.req_trx.ids);
         break;
      case normal :
         c->txn_send(msg.req_trx.ids);
         break;
      case none :
         if(msg.req_blocks.mode == none)
            c->stop_send();
         break;
      default:;
      }
}
```
当nodeos(B)生成一个request_message请求的时候要带上请求的mode，当nodeos(A)收到request_message后，会根据mode来判断对方的意图，来作相应的处理。比如A收到msg.req_blocks.mode为catch_up时，会调用blk_send_branch()将本地分支的不可逆区块与头区块之间的signed_block发送给B；而当msg.req_blocks.mode为normal时，A会调用c->blk_send(msg.req_blocks.ids)，遍历msg.req_blocks.ids中的block id，根据id查询本地是否存在，存在的话就将该signed_block发送给B。其他msg.req_trx.mode也有相应的处理，略过。

##### notice_message

```
void net_plugin_impl::handle_message( connection_ptr c, const notice_message &msg) {
      request_message req;
      bool send_req = false;
      ...
      switch (msg.known_blocks.mode) {
      case none : {
         if (msg.known_trx.mode != normal) {
            return;
         }
         break;
      }
      case last_irr_catch_up:
      case catch_up: {
         sync_master->recv_notice(c,msg);
         break;
      }
      case normal : {
         dispatcher->recv_notice (c, msg, false);
         break;
      }
      default:
      }
      if( send_req) {
         c->enqueue(req);
      }
   }
```
同request_message一样，notice_message也有mode，也会根据不同的mode处理不同的请求。比如A收到的msg.known_blocks.mode为last_irr_catch_up，则A会在sync_master->recv_notice(c,msg)方法中调用sync_manager::start_sync()，向B请求同步区块；而当msg.known_blocks.mode为catch_up时，则A会根据B发送过来的msg判断分支情况，再生成一个相应mode的request_message发送给B。这地方会有点绕，需要几个message结合起来理解一下。

##### sync_request_message

前面分析中提到，根据不同的情况B会发送不同的message给A，其中一种情况就是B在交流中发现自己落伍了，想要跟上大部队的步伐，于是需要从A那里获取区块，此时B调用sync_manager::start_sync()-->sync_manager::request_next_chunk()-->connection::request_sync_blocks()，发送一个sync_request_message给A，并将B的同步状态设为lib_catchup。
sync_request_message比较简单，只有两个参数:start_block/end_block，分别表示要同步块的开始号和结束号，比如想要同步的区块范围是101-1000，而一次同步100个，则start_block/end_block分别设为101和200。
当A收到sync_request_message后，会调用handle_message()-->enqueue_sync_block()从本地查询num为101的signed_block，查询到了就放入队列，然后异步发送给B，发送完后继续调用enqueue_sync_block()，直至200(end_block)结束。
当B收到block_number为200(end_block)的signed_block后，会根据情况判断继续向A请求201-300的块呢，还是结束此次同步，后面会分析。
```
struct sync_request_message {
      uint32_t start_block;
      uint32_t end_block;
   };
void net_plugin_impl::handle_message( connection_ptr c, const sync_request_message &msg) {
      if( msg.end_block == 0) {
         c->peer_requested.reset();
         c->flush_queues();
      } else {
         c->peer_requested = sync_state( msg.start_block,msg.end_block,msg.start_block-1);
         c->enqueue_sync_block();
      }
}
bool connection::enqueue_sync_block() {
      controller& cc = app().find_plugin<chain_plugin>()->chain();
      if (!peer_requested)
         return false;
      uint32_t num = ++peer_requested->last;
      bool trigger_send = num == peer_requested->start_block;
      if(num == peer_requested->end_block) {
         peer_requested.reset();
      }
      try {
         signed_block_ptr sb = cc.fetch_block_by_number(num);
         if(sb) {
            enqueue( *sb, trigger_send);
            return true;
         }
      } catch ( ... ) {
         wlog( "write loop exception" );
      }
      return false;
}
```

##### signed_block

接着上面分析，当B向A发送了sync_request_message，而A在本地查询到signed_block并发给B后，B怎么处理呢。同样的，会交给handle_message()来处理，继续看关键源码。
```
void net_plugin_impl::handle_message( connection_ptr c, const signed_block &msg) {
      controller &cc = chain_plug->chain();
      block_id_type blk_id = msg.id();
      uint32_t blk_num = msg.block_num();
      
      try {
         if( cc.fetch_block_by_id(blk_id)) {
            sync_master->recv_block(c, blk_id, blk_num);
            return;
         }
      } catch( ...) {
      }

      dispatcher->recv_block(c, blk_id, blk_num);

      go_away_reason reason = fatal_other;
      try {
         signed_block_ptr sbp = std::make_shared<signed_block>(msg);
         chain_plug->accept_block(sbp); //, sync_master->is_active(c));
         reason = no_reason;
      } catch( ...) {
      }

      update_block_num ubn(blk_num);
      if( reason == no_reason ) {
         ...
         sync_master->recv_block(c, blk_id, blk_num);
      }
      else {
         sync_master->rejected_block(c, blk_num);
      }
   }
```
从源码可以看到，先在本地查询是否已经存在此ID的block，已经存在的话就调用sync_master->recv_block()，然后return。没有查询到就会调用dispatcher->recv_block()，将收到的block的信息保存到（或者修改）本地缓存。然后，将该signed_block交给chain_plug->accept_block()进行验证等后续操作，这里不深入讲解。如果验证的时候没有抛异常，就会调用sync_master->recv_block()，否则sync_master->rejected_block()。
看一下sync_manager::recv_block()方法的作用，在前面介绍的请求同步块的方法start_sync()中，B已经将同步状态设为lib_catchup，当处于该同步状态时，recv_block()会检测收到的块是不是想要的下一个块，比如B想要的是666，而A发来的是665或者888，那就说明A是来捣乱的，果断关闭A的连接。如果是B想要的，那就判断是不是最终要请求的区块(比如1000)，如果是最终的1000，则更改同步状态为in_sync，并向其他节点广播handshake_message，再来一遍握手交流；如果不是最终的1000，比如是200，则会继续请求下一组区块，比如201-300。
再说下sync_master->rejected_block()的作用，当检测到A发送过来的块是有问题的，说明A还是来捣乱的，那么就在rejected_block()方法中关闭A的连接，设置同步状态为in_sync，并广播handshake_message与其他节点再来一次亲密的握手交流。

##### packed_transaction

关于p2p消息，最后再来讲一下packed_transaction处理，同样看一下handle_message()方法的关键部分。
```
void net_plugin_impl::handle_message( connection_ptr c, const packed_transaction &msg) {
      transaction_id_type tid = msg.id();
      if(local_txns.get<by_id>().find(tid) != local_txns.end()) {
         return;
      }
      dispatcher->recv_transaction(c, tid);
      uint64_t code = 0;
      chain_plug->accept_transaction(msg, [=](const static_variant<fc::exception_ptr, transaction_trace_ptr>& result) {
         if (result.contains<fc::exception_ptr>()) {
         } else {
            auto trace = result.get<transaction_trace_ptr>();
            if (!trace->except) {
               dispatcher->bcast_transaction(msg);
               return;
            }
         }
         dispatcher->rejected_transaction(tid);
      });
   }
```
从源码中看到，首先会查找本地多索引缓存(local_txns)是否已经存在此transaction，已经存在的话就return，未查找到就dispatcher->recv_transaction()，在received_transactions(vector容器)中缓存一下该交易信息，然后会chain_plug->accept_transaction()验证交易的合法性，如果未出现异常，则dispatcher->bcast_transaction(msg)，将该交易保存到本地多索引容器中，并向其他未处于同步状态，也没有广播过此交易信息的连接广播此packed_transaction，其他节点收到后同样流程处理该packed_transaction。

从上面分析来看，不同nodeos之间会通过不同类型的message来表达自己的意图，告知对方自己的区块信息，并请求对方的信息，从而实现彼此之间的通信，同步链的区块/交易数据。

另外，上面介绍packed_transaction的时候，有说到handle_message()方法中验证交易合法后，该节点会向其他节点广播packed_transaction。EOS除了在handle_message()处理中进行交易的广播外，还采用boost signal绑定的方式，在验证交易或者区块合法后，通过emit方法将该交易或者区块向其他节点进行广播。比如nodeos将cleos发送过来transaction进行验证，验证通过后，会在本地缓存该交易并向其他节点广播这条交易，而正在出块的生产者，负责把该时段内的交易打包到区块中，生成新的区块，然后将该区块也广播出去，让其他的出块节点进行验证，区块验证采用DPoS共识+BFT算法，简单的说就是，如果验证通过的节点超过总出块节点的2/3，则该块就会变为不可逆区块。

----------------------
## HTTP网络

EOS客户端和服务器端的通信采用REST API方式，服务器端的每个资源或者合约执行对应一个唯一的URL地址，客户端将URL地址封装成http请求发送到服务器，请求对应的资源或者执行相应的操作。

### 客户端(cleos)

客户端是通过cleos命令行工具创建主命令、子命令和选项，并解析用户参数，然后调用对应命令的回调函数与nodeos/wallet进行交互，以转账为例，分析一下通信流程。
首先，假设已经设置了eosio.token合约
```
// Transfer subcommand
string con = "eosio.token";
auto transfer = app.add_subcommand("transfer", localized("Transfer EOS from account to account"), false);
transfer->add_option("sender", sender, localized("The account sending EOS"))->required();
transfer->add_option("recipient", recipient, localized("The account receiving EOS"))->required();
transfer->add_option("amount", amount, localized("The amount of EOS to send"))->required();
transfer->add_option("memo", memo, localized("The memo for the transfer"));
transfer->add_option("--contract,-c", con, localized("The contract which controls the token"));

add_standard_transaction_options(transfer, "sender@active");
transfer->set_callback([&] {
  signed_transaction trx;
  if (tx_force_unique && memo.size() == 0) {
     // use the memo to add a nonce
     memo = generate_nonce_string();
     tx_force_unique = false;
  }
  send_actions({create_transfer(con,sender, recipient, to_asset(amount), memo)});
});
```
上面代码中，主要分析send_actions({create_transfer(con,sender, recipient, to_asset(amount), memo)});
create_transfer生成一个chain::action对象，包括转账信息/权限等，然后send_actions()方法中调用push_actions()，将actions放入signed_transaction对象中，并调用push_transaction()方法，完成交易的打包，再调用call方法与服务器通信。
```
void send_actions(...) {
	auto result = push_actions( move(actions), extra_kcpu, compression);
}
fc::variant push_actions(...) {
   signed_transaction trx;
   trx.actions = std::forward<decltype(actions)>(actions);
   return push_transaction(trx, extra_kcpu, compression);
}
fc::variant push_transaction(...) {
	auto info = get_info();
    trx.expiration = info.head_block_time + tx_expiration;
    trx.set_reference_block(ref_block_id);
    trx.max_cpu_usage_ms = tx_max_net_usage;
    trx.max_net_usage_words = (tx_max_net_usage + 7)/8;
    if (!tx_skip_sign) {
        auto required_keys = determine_required_keys(trx);
        sign_transaction(trx, required_keys, info.chain_id);
    }
    if (!tx_dont_broadcast) {
    	//push_txn_func: "/v1/chain/push_transaction"
    	return call(push_txn_func, packed_transaction(trx, compression));
    } else {
    	return fc::variant(trx);
    }
}
fc::variant call( const std::string& url, const std::string& path, const T& v ) {
	return eosio::client::http::do_http_call( *cp, fc::variant(v), print_request, print_response );
}
fc::variant do_http_call(...){
	...
    re = do_txrx(socket, request, status_code);
}
```
在上面push_transaction()方法中，首先通过get_info()与服务器交互获取当前链信息，然后生成交易过期时间，设置引用的块号，max_cpu_usage_ms等，如果需要签名的话，调用sign_transaction()，与wallet交互进行签名，然后调用call方法，传入参数push_txn_func("/v1/chain/push_transaction")等。在call方法中调用do_http_call()完成对http请求的封装，然后调用do_txrx()方法发送封装好的http请求，并等待服务器返回结果。

### 服务器(nodeos)

首先服务器启动的时候，会调用chain_api_plugin的plugin_startup()，将url_handler注册到http_plugin_impl的map<string,url_handler>  url_handlers中，并监听http连接请求。
```
void chain_api_plugin::plugin_startup() {
   app().get_plugin<http_plugin>().add_api({
      CHAIN_RO_CALL(get_info, 200l),
      CHAIN_RO_CALL(get_block, 200),
      CHAIN_RO_CALL(get_block_header_state, 200),
      CHAIN_RO_CALL(get_account, 200),
      CHAIN_RO_CALL(get_code, 200),
      CHAIN_RO_CALL(get_abi, 200),
      CHAIN_RO_CALL(get_table_rows, 200),
      CHAIN_RO_CALL(get_currency_balance, 200),
      CHAIN_RO_CALL(get_currency_stats, 200),
      CHAIN_RO_CALL(get_producers, 200),
      CHAIN_RO_CALL(abi_json_to_bin, 200),
      CHAIN_RO_CALL(abi_bin_to_json, 200),
      CHAIN_RO_CALL(get_required_keys, 200),
      CHAIN_RW_CALL_ASYNC(push_block, chain_apis::read_write::push_block_results, 202),
      CHAIN_RW_CALL_ASYNC(push_transaction, chain_apis::read_write::push_transaction_results, 202),
      CHAIN_RW_CALL_ASYNC(push_transactions, chain_apis::read_write::push_transactions_results, 202)
   });
}
void add_api(const api_description& api) {
   for (const auto& call : api) 
      add_handler(call.first, call.second);
}
void http_plugin::add_handler(const string& url, const url_handler& handler) {
  app().get_io_service().post([=](){
    my->url_handlers.insert(std::make_pair(url,handler));
  });
}
```
当有客户端连接服务器，服务器通过http_plugin接收客户端的http请求，然后解析出请求的URL地址（resource）和消息内容（body），然后调用url_handlers中对应的回调函数进行处理，并将结果返回给cleos客户端。在转账这个例子中，会调用push_transaction()方法完成交易信息的验证/缓存/广播等后续处理，并将结果返回给客户端
```
void handle_http_request(...){
	auto body = con->get_request_body();
    auto resource = con->get_uri()->get_resource();
    auto handler_itr = url_handlers.find( resource );
    if( handler_itr != url_handlers.end()) {
      con->defer_http_response();
      handler_itr->second( resource, body, [con]( auto code, auto&& body ) {
         con->set_body( std::move( body ));
         con->set_status( websocketpp::http::status_code::value( code ));
         con->send_http_response();
      } );
    } 
}

```