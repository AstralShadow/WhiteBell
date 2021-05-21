#ifndef CLIENT_PARSER_H
#define CLIENT_PARSER_H

#include "Client.h"
#include <cstdint>
#include <vector>

using std::vector;

class Server;
class InputBuffer;

class Client::Parser
{
    public:
        Parser(Client*);
        Parser(const Parser&) = delete;
        void operator = (const Parser&) = delete;
        virtual ~Parser() = default;

        bool can_parse() const;
        void parse();

    protected:

    private:
        Client* client;
        InputBuffer* input;

        enum struct Step{
            header,
            name_length,
            payload_length,
            name,
            payload,
            process
        };
        enum struct OPCode{
            set_namespace = 0,
            track_event = 1,
            untrack_event = 2,
            trigger_event = 3,
            track_counter = 4,
            untrack_counter = 5,
            join_counter = 6,
            leave_counter = 7,
            undefined = 16
        };

        Step step = Step::header;

        uint8_t header = 0;
        OPCode opcode = OPCode::undefined;
        uint8_t name_length = 0;
        uint16_t payload_length = 0;
        vector<uint8_t> name = {};
        vector<uint8_t> payload = {};

        size_t required_input_size() const;
        void increase_step();
        void parse_header();
        void read_name_length();
        void read_payload_length();
        void read_name();
        void read_payload();
        void process_packet();

};

#endif // CLIENT_PARSER_H
