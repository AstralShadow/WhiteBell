#include "../include/cstring_t.hpp"
#include "../include/Client.hpp"
#include "../include/Client_Details.hpp"
#include "../include/OPCode.hpp"
#include "../include/InputBuffer.hpp"

namespace WhiteBell
{
    const uint8_t protocol_version = 1;

    bool Client::Details::can_process_input() const
    {
        size_t avaliable = this->buffer->avaliable();
        auto step = this->step;

        switch(step)
        {
            case processing_step::header:
                return avaliable >= 1;

            case processing_step::protocol_version:
                return avaliable >= 1;

            case processing_step::name_length:
                return avaliable >= 1;

            case processing_step::counter_value:
                return avaliable >= 4;

            case processing_step::payload_length:
                return avaliable >= 2;

            case processing_step::name:
                return avaliable >= this->name_length;

            case processing_step::payload:
                return avaliable >= this->payload_length;

            case processing_step::call_counter_listeners:
                return true;
            case processing_step::call_event_listeners:
                return true;

            default: return false;
        }
    }

    void Client::Details::process_input()
    {
        switch(this->step)
        {
            case processing_step::header:
                this->process_header();
                return;

            case processing_step::protocol_version:
                this->process_version();
                return;

            case processing_step::name_length:
                this->process_name_length();
                return;

            case processing_step::counter_value:
                this->process_counter_value();
                return;

            case processing_step::payload_length:
                this->process_payload_length();
                return;

            case processing_step::name:
                this->process_name();
                return;

            case processing_step::payload:
                this->process_payload();
                return;

            case processing_step::call_counter_listeners:
                this->call_counter_listeners(this->name, this->counter_value);
                this->step = processing_step::header;
                return;

            case processing_step::call_event_listeners:
                this->call_event_listeners(this->name, this->payload);
                this->step = processing_step::header;
                return;
        }
    }

    void Client::Details::process_header()
    {
        uint8_t header = this->buffer->get(1)[0];
        this->opcode = static_cast<OPCode::server_code>(header & 15);
        switch(this->opcode)
        {
            case OPCode::protocol_version:
                this->step = processing_step::protocol_version;
                break;

            case OPCode::event_triggered:
            case OPCode::counter_changed:
                this->step = processing_step::name_length;
                break;
        }
    }

    void Client::Details::process_version()
    {
        uint8_t version = this->buffer->get(1)[0];
        if(version != WhiteBell::protocol_version)
            throw UnsupportedProtocolException();
        this->step = processing_step::header;
    }

    void Client::Details::process_name_length()
    {
        this->name_length = this->buffer->get(1)[0];
        switch(this->opcode)
        {
            case OPCode::protocol_version: break;

            case OPCode::event_triggered:
                this->step = processing_step::payload_length;
                break;

            case OPCode::counter_changed:
                this->step = processing_step::counter_value;
                break;
        }
    }

    void Client::Details::process_counter_value()
    {
        auto input = this->buffer->get(4);
        uint32_t value = 0;
        for(int i = 0; i < 4; i++){
            value = (value << 8) | input[i];
        }
        this->counter_value = value;

        this->step = processing_step::name;
    }

    void Client::Details::process_payload_length()
    {
        auto input = this->buffer->get(2);
        uint16_t value = 0;
        for(int i = 0; i < 2; i++){
            value = (value << 8) | input[i];
        }
        this->payload_length = value;

        this->step = processing_step::name;
    }

    void Client::Details::process_name()
    {
        auto input = this->buffer->get(this->name_length);
        char* begin = reinterpret_cast<char*>(&input[0]);
        this->name = cstring_t(begin, input.size());

        switch(this->opcode)
        {
            case OPCode::protocol_version: break;

            case OPCode::event_triggered:
                this->step = processing_step::payload;
                break;

            case OPCode::counter_changed:
                this->step = processing_step::call_counter_listeners;
                break;
        }
    }

    void Client::Details::process_payload()
    {
        auto input = this->buffer->get(this->payload_length);
        char* begin = reinterpret_cast<char*>(&input[0]);
        this->payload = cstring_t(begin, input.size());

        this->step = processing_step::call_event_listeners;
    }


}
