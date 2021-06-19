#ifndef CLIENT_DETAILS_HPP_INCLUDED
#define CLIENT_DETAILS_HPP_INCLUDED

#include "cstring_t.hpp"
#include "Client.hpp"
#include "OPCode.hpp"
#include <sys/socket.h>
#include <vector>
#include <map>
#include <string>

using std::vector;
using std::map;
using std::string;
struct sockaddr;


namespace WhiteBell {
    class InputBuffer;
    typedef void(*event_cb_t)(cstring_t);
    typedef void(*counter_cb_t)(uint32_t);

    namespace processing_step
    {
        enum value
        {
            header,
            protocol_version,
            name_length,
            counter_value,
            payload_length,
            name,
            payload,
            call_counter_listeners,
            call_event_listeners
        };
    }

    class Client::Details
    {
        public:
            Details(cstring_t const& path);
            Details(cstring_t const& ip, uint16_t const& port);
            virtual ~Details();

            bool running = false;

            typedef map<std::string, vector<event_cb_t>> event_listeners_t;
            typedef map<std::string, vector<counter_cb_t>> counter_listeners_t;
            event_listeners_t event_listeners;
            counter_listeners_t counter_listeners;

            ssize_t get_fd() const;
            void send(cstring_t const&);
            void send(vector<char> const&);
            void send(OPCode::client_code code, cstring_t const&);
            void send(OPCode::client_code code, cstring_t const&, cstring_t const&);
            void receive();
            void receive_non_block();


        private:
            ssize_t fd;
            sockaddr* addr;

            InputBuffer* buffer;

            processing_step::value step = processing_step::header;
            OPCode::server_code opcode;
            uint8_t name_length;
            cstring_t name;
            uint16_t payload_length;
            cstring_t payload;
            uint32_t counter_value;

            bool can_process_input() const;
            void process_input();

            void process_header();
            void process_version();
            void process_name_length();
            void process_counter_value();
            void process_payload_length();
            void process_name();
            void process_payload();

            void call_counter_listeners(cstring_t const& name, uint32_t counter_value);
            void call_event_listeners(cstring_t const& name, cstring_t const& payload);
    };
}

#endif // CLIENT_DETAILS_HPP_INCLUDED
