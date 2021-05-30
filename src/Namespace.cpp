#include "Namespace.h"
#include "Client.h"
#include <algorithm>

using std::find;

Namespace::Namespace(name_t name) :
    _name(name)
{
    // ctor
}

name_t Namespace::get_name() const
{
    return this->_name;
}

void Namespace::add_client(shared_ptr<Client> client)
{
    auto events = client->get_tracked_events();
    for(auto v_itr = events.begin(); v_itr != events.end(); ++v_itr){
        this->add_event_listener(*v_itr, client);
    }

    auto counter = client->get_tracked_counters();
    for(auto v_itr = counter.begin(); v_itr != counter.end(); ++v_itr){
        this->add_counter_listener(*v_itr, client);
    }

    auto joined = client->get_joined_counters();
    for(auto v_itr = joined.begin(); v_itr != joined.end(); ++v_itr){
        this->join_counter(*v_itr, client);
    }
}

void Namespace::remove_client(shared_ptr<Client> client)
{
    auto events = client->get_tracked_events();
    for(auto v_itr = events.begin(); v_itr != events.end(); ++v_itr){
        this->remove_event_listener(*v_itr, client);
    }

    auto counter = client->get_tracked_counters();
    for(auto v_itr = counter.begin(); v_itr != counter.end(); ++v_itr){
        this->remove_counter_listener(*v_itr, client);
    }

    auto joined = client->get_joined_counters();
    for(auto v_itr = joined.begin(); v_itr != joined.end(); ++v_itr){
        this->leave_counter(*v_itr, client);
    }
}

void Namespace::add_event_listener(const name_t& name, shared_ptr<Client> client)
{
    this->event_listeners[name].push_back(weak_ptr<Client>(client));
}

void Namespace::remove_event_listener(const name_t& name, shared_ptr<Client> client)
{
    auto map_itr = this->event_listeners.find(name);
    if(map_itr == this->event_listeners.end())
        return;

    auto &container = map_itr->second;
    auto itr = container.begin();
    for(; itr != container.end(); ++itr){
        if(client.owner_before(*itr))
            continue;
        if(itr->owner_before(client))
            continue;
        break;
    }
    container.erase(itr);

    if(container.size() == 0){
        this->event_listeners.erase(map_itr);
    }
}

void Namespace::add_counter_listener(const name_t& name, shared_ptr<Client> client)
{
    auto& counter = this->counter_listeners[name];
    counter.push_back(weak_ptr<Client>(client));

    uint32_t value = 0;

    auto counters_itr = this->counters.find(name);
    if(counters_itr != this->counters.end())
        value = counters_itr->second.size();

    client->send_counter_event(name, value);
}

void Namespace::remove_counter_listener(const name_t& name, shared_ptr<Client> client)
{
    auto map_itr = this->counter_listeners.find(name);
    if(map_itr == this->counter_listeners.end())
        return;

    auto &container = map_itr->second;
    auto itr = container.begin();
    for(; itr != container.end(); ++itr){
        if(client.owner_before(*itr))
            continue;
        if(itr->owner_before(client))
            continue;
        break;
    }
    container.erase(itr);

    if(container.size() == 0){
        this->counter_listeners.erase(map_itr);
    }
}

void Namespace::join_counter(const name_t& name, shared_ptr<Client> client)
{
    this->counters[name].push_back(weak_ptr<Client>(client));
    this->trigger_counter_event(name);
}

void Namespace::leave_counter(const name_t& name, shared_ptr<Client> client)
{
    auto map_itr = this->counters.find(name);
    if(map_itr == this->counters.end())
        return;

    auto &container = map_itr->second;
    auto itr = container.begin();
    for(; itr != container.end(); ++itr){
        if(client.owner_before(*itr))
            continue;
        if(itr->owner_before(client))
            continue;
        break;
    }
    container.erase(itr);

    if(container.size() == 0){
        this->counters.erase(map_itr);
    }

    this->trigger_counter_event(name);
}

void Namespace::trigger_event(const name_t& name, const payload_t& payload)
{
    auto map_itr = this->event_listeners.find(name);
    if(map_itr == this->event_listeners.end())
        return;

    auto& listeners = map_itr->second;
    for(auto itr = listeners.begin(); itr != listeners.end(); ++itr){
        if(auto client = itr->lock()){
            client->send_event(name, payload);
        }
    }
}

void Namespace::trigger_counter_event(const name_t& name)
{
    auto map_itr = this->counter_listeners.find(name);
    if(map_itr == this->counter_listeners.end())
        return;

    uint32_t value = 0;
    auto& listeners = map_itr->second;

    auto counters_itr = this->counters.find(name);
    if(counters_itr != this->counters.end())
        value = counters_itr->second.size();

    uint8_t value_bits[4];
    for(int8_t i = 3; i >= 0; i--){
        value_bits[3 - i] = value >> (i * 8);
    }


    for(auto itr = listeners.begin(); itr != listeners.end(); ++itr){
        if(auto client = itr->lock()){
            client->send_counter_event(name, value_bits);
        }
    }
}
