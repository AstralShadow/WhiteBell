#include "../include/WhiteBellClient.hpp"
#include "../include/Client_Details.hpp"
#include "../include/OPCode.hpp"
#include <sys/select.h>
#include <vector>
#include <map>
#include <algorithm>

using std::vector;
using std::map;
using std::find;

namespace WhiteBell
{
    Client::Client(cstring_t const& path) :
        details(new Details(path))
    {

    }

    Client::Client(cstring_t const& ip, uint16_t const& port) :
        details(new Details(ip, port))
    {

    }

    Client::~Client()
    {
        delete this->details;
    }

    void Client::fetch()
    {
        this->details->receive_non_block();
    }

    void Client::run()
    {
        this->details->running = true;
        while(this->details->running){
            this->details->receive();
        }
    }

    void Client::stop()
    {
        this->details->running = false;
    }


    void Client::set_namespace(cstring_t const& name)
    {
        this->details->send(OPCode::set_namespace, name);
    }

    void Client::track_event(cstring_t const& name, event_cb_t cb)
    {
        auto &listeners_map = this->details->event_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            this->details->send(OPCode::track_event, name);
        }
        listeners_map[name].push_back(cb);
    }

    void Client::untrack_event(cstring_t const& name, event_cb_t cb)
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

    void Client::untrack_event(cstring_t const& name)
    {
        this->details->send(OPCode::untrack_event, name);

        auto &listeners_map = this->details->event_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            return;
        }

        listeners_map.erase(map_itr);
    }

    void Client::trigger_event(cstring_t const& name, cstring_t const& payload)
    {
        this->details->send(OPCode::trigger_event, name, payload);
    }

    void Client::track_counter(cstring_t const& name, counter_cb_t cb)
    {
        auto &listeners_map = this->details->counter_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            this->details->send(OPCode::track_counter, name);
        }
        listeners_map[name].push_back(cb);
    }

    void Client::untrack_counter(cstring_t const& name, counter_cb_t cb)
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

    void Client::untrack_counter(cstring_t const& name)
    {
        this->details->send(OPCode::untrack_counter, name);

        auto &listeners_map = this->details->counter_listeners;
        auto map_itr = listeners_map.find(name);
        if(map_itr == listeners_map.end()){
            return;
        }

        listeners_map.erase(map_itr);
    }

    void Client::join_counter(cstring_t const& name)
    {
        this->details->send(OPCode::join_counter, name);
    }

    void Client::leave_counter(cstring_t const& name)
    {
        this->details->send(OPCode::leave_counter, name);
    }
}
