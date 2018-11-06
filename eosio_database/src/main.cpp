#include <iostream>
#include <fstream>
#include <database/database.hpp>
#include <protocol/sample.pb.h>
#include <google/protobuf/service.h>
#include <rpc_service/utility.hpp>
#include <thread>
#include <protocol/fork_db_message_wrapper.hpp>


void prompt_for_address( tutorial::person *person ) {
    std::cout << "Enter person ID number: ";
    int id;
    std::cin >> id;
    person->set_id( id );
    std::cin.ignore( 256,'\n' );

    std::cout << "Enter name: ";
    std::getline(std::cin,*person->mutable_name());

    std::cout << "Enter email address (blank for none): ";
    std::string email;
    std::getline(std::cin,email);
    if(!email.empty())
        person->set_email(email);

    while(true) {
        std::cout << "Enter a phone number (or leave blank to finish): ";
        std::string number;
        std::getline(std::cin,number);
        if(number.empty())
            break;

        tutorial::person::phone_number *phone_number = person->add_phones();
        phone_number->set_number(number);
        std::cout << "Is this a mobile,home,or work phone? ";
        std::string type;
        std::getline(std::cin,type);
        if(type == "mobile") {
            phone_number->set_type(tutorial::person::MOBILE);
        } else if(type == "home") {
            phone_number->set_type(tutorial::person::HOME);
        } else if(type == "work") {
            phone_number->set_type(tutorial::person::WORK);
        } else {
            std::cout << "Unknown phone type. Using default." << std::endl;
        }
    }
}

void list_people( const tutorial::address_book &book ) {
    for( int i = 0;i < book.people_size();++i ) {
        const tutorial::person &person = book.people(i);
        std::cout << "Person ID: " << person.id() << std::endl;
        std::cout << "Name: " << person.name() << std::endl;
        if(person.has_email()) {
            std::cout << "E-mail address: " << person.email() << std::endl;
        }

        for(int j = 0;j < person.phones_size();++j) {
            const tutorial::person::phone_number &number = person.phones(j);
            switch (number.type()) {
                case tutorial::person::MOBILE:
                    std::cout << "Mobile phone #: ";
                    break;
                case tutorial::person::HOME:
                    std::cout << "Home phone #: ";
                    break;
                case tutorial::person::WORK:
                    std::cout << "Work phone #: ";
                    break;
            }
            std::cout << number.number() << std::endl;
        }
    }
}

using namespace google::protobuf;

class service_impl : public tutorial::address_book_service {
public:
};

class server {
public:
    server(google::protobuf::Service* imp):_service{imp} {}
    void init_service() {
        const google::protobuf::ServiceDescriptor *server_desc = _service->GetDescriptor();
        int method_count = server_desc->method_count();
        for(int i = 0;i < method_count;++i) {
            auto method = server_desc->method(i);
            std::cout << "short name: " << method->name() << " full name: " << method->full_name() << std::endl;
        }
    }

private:
    google::protobuf::Service* _service;
};

int main(int argc, char const *argv[]) {

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    auto request = get_block_add_by_block_state_request();
    request->set_data("aaaa");
    char buf[32]{};
    std::size_t ret_len = request->get_data(buf,32);
    int32_t len;
    std::memcpy(&len,buf,sizeof(int32_t));
    char *data = new char[len];
    std::memcpy(data,buf+ sizeof(int32_t),len);

    auto req = get_raw_block_add_by_block_state_request();
    req->ParseFromArray(data,len);
    std::cout << req->data() << std::endl;

    std::cout << std::thread::hardware_concurrency() << std::endl;

    hash_type<std::string> hasher("tutorial.get_person");
    std::cout << hasher.hash() << std::endl;

    service_impl ser_impl;
    server serv(&ser_impl);
    serv.init_service();

    const char *contracts = "./contracts.txt";

    tutorial::address_book book;
    {
        std::fstream input(contracts,std::ios::in | std::ios::binary);
        if(!input) {
            std::cout << contracts << ": File not found. Creating a new file" << std::endl;
        } else if( !book.ParseFromIstream(&input) ) {
            std::cerr << "Failed to parse address book." << std::endl;
            return -1;
        }
    }

    prompt_for_address(book.add_people());
    {
        std::fstream output(contracts,std::ios::out | std::ios::trunc | std::ios::binary);
        if(!book.SerializeToOstream(&output)) {
            std::cerr << "Failed to write address book." << std::endl;
            return -1;
        }

    }


    list_people(book);

    google::protobuf::ShutdownProtobufLibrary();

    service_impl serve;

    std::cout << "Hello,World" << std::endl;
    return 0;
}
