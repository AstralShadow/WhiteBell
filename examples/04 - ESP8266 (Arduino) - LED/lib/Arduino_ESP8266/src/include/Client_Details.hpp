#ifndef CLIENT_DETAILS_HPP_INCLUDED
#define CLIENT_DETAILS_HPP_INCLUDED

#include "Client.hpp"
#include "OPCode.hpp"
#include <WiFiClient.h>
#include <vector>
#include <map>
#include <string>

using std::vector;
using std::string;


namespace WhiteBell {

    typedef void(*event_cb_t)(string);
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
            Details(string const& ip, uint16_t const& port);
            virtual ~Details() = default;

            bool running = false;

            typedef std::map<string, vector<event_cb_t>> event_listeners_t;
            typedef std::map<string, vector<counter_cb_t>> counter_listeners_t;
            event_listeners_t event_listeners;
            counter_listeners_t counter_listeners;

            void send(string const&);
            void send(vector<char> const&);
            void send(OPCode::client_code code, string const&);
            void send(OPCode::client_code code, string const&, string const&);
            void process();


        private:
            WiFiClient client;

            processing_step::value step = processing_step::header;
            OPCode::server_code opcode;
            uint8_t name_length;
            string name;
            uint16_t payload_length;
            string payload;
            uint32_t counter_value;

            bool can_process_input();
            void process_input();

            void process_header();
            void process_version();
            void process_name_length();
            void process_counter_value();
            void process_payload_length();
            void process_name();
            void process_payload();

            void call_counter_listeners(string const& name, uint32_t counter_value);
            void call_event_listeners(string const& name, string const& payload);
    };
}

#endif // CLIENT_DETAILS_HPP_INCLUDED
