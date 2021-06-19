#include "../WhiteBellClient.hpp"
#include "../include/Client_Details.hpp"
#include <WiFiClient.h>
#include <string>
#include <memory>
#include <vector>
#include <map>

using std::string;
using std::copy;
using std::vector;


namespace WhiteBell
{
    Client::Details::Details(string const& ip, uint16_t const& port)
    {
        this->client.connect(ip.c_str(), port);
    }

    void Client::Details::send(string const& input)
    {
        this->client.write(input.c_str(), input.size());
    }

    void Client::Details::send(vector<char> const& input)
    {
        this->client.write(&input[0], input.size());
    }

    void Client::Details::send(OPCode::client_code code, string const& name){
        vector<char> output;
        output.reserve(2 + name.size());

        char header = static_cast<uint8_t>(code);
        output.push_back(header);

        output.push_back(name.size());
        output.insert(output.end(), name.begin(), name.end());

        this->send(output);
    }

    void Client::Details::send(OPCode::client_code code, string const& name, string const& payload){
        vector<char> output;
        output.reserve(4 + name.size() + payload.size());

        char header = static_cast<uint8_t>(code);
        output.push_back(header);

        output.push_back(name.size());
        uint16_t p_size = payload.size();
        output.push_back(p_size >> 8);
        output.push_back(p_size & 255);
        output.insert(output.end(), name.begin(), name.end());
        output.insert(output.end(), payload.begin(), payload.end());

        this->send(output);
    }

    void Client::Details::process()
    {
        while(this->can_process_input()){
            this->process_input();
        }
    }

    void Client::Details::call_counter_listeners
        (string const& name, uint32_t value)
    {
        auto& collections = this->counter_listeners;
        auto map_itr = collections.find(name);
        if(map_itr == collections.end())
            return;

        auto listeners = map_itr->second;
        auto itr = listeners.begin();
        while(itr != listeners.end()){
            (*itr)(value);
            ++itr;
        }
    }

    void Client::Details::call_event_listeners
        (string const& name, string const& payload)
    {
        auto& collections = this->event_listeners;
        auto map_itr = collections.find(name);
        if(map_itr == collections.end()){
            return;
        }

        auto listeners = map_itr->second;
        auto itr = listeners.begin();
        while(itr != listeners.end()){
            (*itr)(payload);
            ++itr;
        }
    }
}
