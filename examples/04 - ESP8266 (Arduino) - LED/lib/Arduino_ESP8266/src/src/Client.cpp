#include "../include/Client.hpp"
#include "../include/Client_Details.hpp"
#include "../include/OPCode.hpp"
#include <WiFiClient.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using std::string;
using std::vector;
using std::find;

namespace WhiteBell
{
    Client::Client(string const& ip, uint16_t const& port) :
        details(new Details(ip, port))
    {

    }

    Client::~Client()
    {
        delete this->details;
    }

    void Client::fetch()
    {
        this->details->process();
    }

    void Client::set_namespace(string const& name)
    {
        this->details->send(OPCode::set_namespace, name);
    }

    void Client::track_event(string const& name, event_cb_t cb)
    {
        auto &listeners_map = this->details->event_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            this->details->send(OPCode::track_event, name);
        }
        listeners_map[name].push_back(cb);
    }

    void Client::untrack_event(string const& name, event_cb_t cb)
    {
        auto &listeners_map = this->details->event_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            return;
        }

        auto &listeners = map_itr->second;
        auto itr = find(listeners.begin(), listeners.end(), cb);
        listeners.erase(itr);
        if(listeners.size() == 0){
            listeners_map.erase(map_itr);
            this->details->send(OPCode::untrack_event, name);
        }
    }

    void Client::untrack_event(string const& name)
    {
        this->details->send(OPCode::untrack_event, name);

        auto &listeners_map = this->details->event_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            return;
        }

        listeners_map.erase(map_itr);
    }

    void Client::trigger_event(string const& name, string const& payload)
    {
        this->details->send(OPCode::trigger_event, name, payload);
    }

    void Client::track_counter(string const& name, counter_cb_t cb)
    {
        auto &listeners_map = this->details->counter_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            this->details->send(OPCode::track_counter, name);
        }
        listeners_map[name].push_back(cb);
    }

    void Client::untrack_counter(string const& name, counter_cb_t cb)
    {
        auto &listeners_map = this->details->counter_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            return;
        }

        auto &listeners = map_itr->second;
        auto itr = find(listeners.begin(), listeners.end(), cb);
        listeners.erase(itr);
        if(listeners.size() == 0){
            listeners_map.erase(map_itr);
            this->details->send(OPCode::untrack_counter, name);
        }
    }

    void Client::untrack_counter(string const& name)
    {
        this->details->send(OPCode::untrack_counter, name);

        auto &listeners_map = this->details->counter_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            return;
        }

        listeners_map.erase(map_itr);
    }

    void Client::join_counter(string const& name)
    {
        this->details->send(OPCode::join_counter, name);
    }

    void Client::leave_counter(string const& name)
    {
        this->details->send(OPCode::leave_counter, name);
    }
}
