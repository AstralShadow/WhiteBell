#include "Client.h"
#include "Client_InputBuffer.h"
#include "Client_DisconnectedException.h"
#include "Client_Parser.h"
#include "Server.h"
#include "Namespace.h"
#include <algorithm>
#include <unistd.h>

using std::find;
#include <iostream>

const uint8_t Client::protocol_version = 1;

Client::Client(ssize_t _fd, Server* _server) :
    fd(_fd),
    server(_server),
    tracked_events(),
    tracked_counters(),
    joined_counters()
{
    this->buffer = new Client::InputBuffer();
    this->parser = new Client::Parser(this);
    this->send_protocol_version();
}

Client::~Client()
{
    delete this->buffer;
    delete this->parser;
    delete this->_self;
    ::shutdown(this->get_fd(), SHUT_WR);
    close(this->get_fd());
}

ssize_t Client::get_fd() const
{
    return this->fd;
}

shared_ptr<Client> Client::get_ptr()
{
    if(this->_self == nullptr){
        shared_ptr<Client> sp(this);
        this->_self = new weak_ptr<Client>(sp);
        return sp;
    }
    return this->_self->lock();
}

shared_ptr<Namespace> Client::get_namespace() const
{
    return this->_namespace;
}

vector<name_t> Client::get_tracked_events() const
{
    return this->tracked_events;
}

vector<name_t> Client::get_tracked_counters() const
{
    return this->tracked_counters;
}

vector<name_t> Client::get_joined_counters() const
{
    return this->joined_counters;
}

void Client::receive_input()
{
    this->buffer->receive(this->fd);
}

void Client::parse_input()
{
    while(this->parser->can_parse()){
        this->parser->parse();
    }
}

void Client::set_namespace(name_t name)
{
    auto self = this->get_ptr();
    if(this->_namespace){
        this->_namespace->remove_client(self);
    }
    this->_namespace = this->server->get_namespace(name);
    this->_namespace->add_client(self);
}

void Client::track_event(name_t name)
{
    auto& ev_list = this->tracked_events;
    auto itr = std::find(ev_list.begin(), ev_list.end(), name);
    if(itr != ev_list.end()){
        return;
    }
    ev_list.push_back(name);

    if(this->_namespace){
        auto self = this->get_ptr();
        this->_namespace->add_event_listener(name, self);
    }
}

void Client::untrack_event(name_t name)
{
    auto& ev_list = this->tracked_events;
    auto itr = std::find(ev_list.begin(), ev_list.end(), name);
    if(itr == ev_list.end()){
        return;
    }
    ev_list.erase(itr);

    if(this->_namespace){
        auto self = this->get_ptr();
        this->_namespace->remove_event_listener(name, self);
    }
}

void Client::trigger_event(name_t name, payload_t payload)
{
    if(this->_namespace){
        this->_namespace->trigger_event(name, payload);
    }
}

void Client::track_counter(name_t name)
{
    auto& c_list = this->tracked_counters;
    auto itr = std::find(c_list.begin(), c_list.end(), name);
    if(itr != c_list.end()){
        return;
    }
    c_list.push_back(name);

    if(this->_namespace){
        auto self = this->get_ptr();
        this->_namespace->add_counter_listener(name, self);
    }
}

void Client::untrack_counter(name_t name)
{
    auto& tc_list = this->tracked_counters;
    auto itr = std::find(tc_list.begin(), tc_list.end(), name);
    if(itr == tc_list.end()){
        return;
    }
    tc_list.erase(itr);

    if(this->_namespace){
        auto self = this->get_ptr();
        this->_namespace->remove_counter_listener(name, self);
    }
}

void Client::join_counter(name_t name)
{
    auto& jc_list = this->joined_counters;
    auto itr = std::find(jc_list.begin(), jc_list.end(), name);
    if(itr != jc_list.end()){
        return;
    }
    jc_list.push_back(name);

    if(this->_namespace){
        auto self = this->get_ptr();
        this->_namespace->join_counter(name, self);
    }
}

void Client::leave_counter(name_t name)
{
    auto& jc_list = this->joined_counters;
    auto itr = std::find(jc_list.begin(), jc_list.end(), name);
    if(itr == jc_list.end()){
        return;
    }
    jc_list.erase(itr);

    if(this->_namespace){
        auto self = this->get_ptr();
        this->_namespace->leave_counter(name, self);
    }
}

void Client::send(vector<uint8_t> packet)
{
    using ::send;
    ssize_t sent = send(this->fd, &packet[0], packet.size(), 0);
    if(sent < 0){
        perror("Error while sending packet");
        return;
    }
}

void Client::send_protocol_version(){
    vector<uint8_t> packet = {
        static_cast<uint8_t>(OutputOPCode::version),
        this->protocol_version
    };
    this->send(packet);
}

void Client::send_event(const name_t& name, const payload_t& payload){
    vector<uint8_t> packet;
    packet.reserve(4 + name.size() + payload.size());
    auto opcode = OutputOPCode::event_triggered;
    packet.push_back(static_cast<uint8_t>(opcode));
    packet.push_back(name.size());
    packet.push_back(payload.size() >> 8);
    packet.push_back(payload.size() & 255);
    packet.insert(packet.end(), name.begin(), name.end());
    packet.insert(packet.end(), payload.begin(), payload.end());

    this->send(packet);
}

void Client::send_counter_event(const name_t& name, const uint32_t& value){
    vector<uint8_t> packet;
    packet.reserve(6 + name.size());
    auto opcode = OutputOPCode::counter_changed;
    packet.push_back(static_cast<uint8_t>(opcode));
    packet.push_back(name.size());
    for(int8_t i = 3; i >= 0; i--){
        packet.push_back(value >> (i * 8));
    }

    packet.insert(packet.end(), name.begin(), name.end());

    this->send(packet);
}

void Client::send_counter_event(const name_t& name, const uint8_t value[4]){
    vector<uint8_t> packet;
    packet.reserve(6 + name.size());
    auto opcode = OutputOPCode::counter_changed;
    packet.push_back(static_cast<uint8_t>(opcode));
    packet.push_back(name.size());
    for(uint8_t i = 0; i < 4; i++)
        packet.push_back(value[i]);
    packet.insert(packet.end(), name.begin(), name.end());

    this->send(packet);
}



