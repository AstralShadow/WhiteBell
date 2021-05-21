#include "Client.h"
#include "Client_Parser.h"
#include "Client_InputBuffer.h"
#include "Server.h"

Client::Parser::Parser(Client* _client) :
    client(_client),
    input(_client->buffer)
{
    // ctor
}

bool Client::Parser::can_parse() const
{
    size_t needed = this->required_input_size();
    return this->input->avaliable() >= needed;
}

size_t Client::Parser::required_input_size() const
{
    switch(this->step){
        default: return -1;

        case Step::header:
            return 1;

        case Step::name_length:
            return 1;

        case Step::payload_length:
            return 2;

        case Step::name:
            return this->name_length;

        case Step::payload:
            return this->payload_length;

        case Step::process:
            return 0;
    }
}

void Client::Parser::parse()
{
    switch(this->step){
        default: return;

        case Step::header:
            this->parse_header();
            break;

        case Step::name_length:
            this->read_name_length();
            break;

        case Step::payload_length:
            this->read_payload_length();
            break;

        case Step::name:
            this->read_name();
            break;

        case Step::payload:
            this->read_payload();
            break;

        case Step::process:
            this->process_packet();
            break;
    }

    this->increase_step();
}

void Client::Parser::increase_step()
{
    switch(this->step){
        default: return;

        case Step::header:
            this->step = Step::name_length;
            break;

        case Step::name_length:
            if(this->opcode == OPCode::trigger_event){
                this->step = Step::payload_length;
                break;
            }
            this->step = Step::name;
            break;

        case Step::payload_length:
            this->step = Step::name;
            break;

        case Step::name:
            if(this->opcode == OPCode::trigger_event){
                this->step = Step::payload;
                break;
            }
            this->step = Step::process;
            break;

        case Step::payload:
            this->step = Step::process;
            break;

        case Step::process:
            this->step = Step::header;
            break;
    }
}

void Client::Parser::parse_header()
{
    this->header = this->input->get(1)[0];
    uint8_t _opcode = this->header & 15;
    if(_opcode > 7){
        this->opcode = OPCode::undefined;
    }else{
        this->opcode = static_cast<OPCode>(_opcode);
    }
}

void Client::Parser::read_name_length()
{
    this->name_length = this->input->get(1)[0];
}

void Client::Parser::read_payload_length()
{
    auto _input = this->input->get(2);
    this->payload_length = _input[0] * 256 + _input[1];
}

void Client::Parser::read_name()
{
    this->name = this->input->get(this->name_length);
}

void Client::Parser::read_payload()
{
    this->payload = this->input->get(this->payload_length);
}

void Client::Parser::process_packet()
{
    switch(this->opcode){
        case OPCode::undefined:
            return;

        case OPCode::set_namespace:
            client->set_namespace(this->name);
            return;

        case OPCode::track_event:
            client->track_event(this->name);
            return;

        case OPCode::untrack_event:
            client->untrack_event(this->name);
            return;

        case OPCode::trigger_event:
            client->trigger_event(this->name, this->payload);
            return;

        case OPCode::track_counter:
            client->track_counter(this->name);
            return;

        case OPCode::untrack_counter:
            client->untrack_counter(this->name);
            return;

        case OPCode::join_counter:
            client->join_counter(this->name);
            return;

        case OPCode::leave_counter:
            client->leave_counter(this->name);
            return;
    }
}

