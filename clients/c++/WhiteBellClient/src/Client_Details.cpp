#include "../include/WhiteBellClient.hpp"
#include "../include/Client_Details.hpp"
#include "../include/InputBuffer.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <memory>
#include <vector>
#include <map>

using std::copy;
using std::vector;
using std::map;

#include <iostream>
using std::cout;
using std::endl;


namespace WhiteBell
{
    Client::Details::Details(cstring_t const& path) :
        buffer(new InputBuffer())
    {
        sockaddr_un* _addr = new sockaddr_un;
        _addr->sun_family = AF_UNIX;

        copy(path.begin(), path.end(), _addr->sun_path);

        this->addr = reinterpret_cast<sockaddr*>(_addr);

        this->fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if(this->fd < 0)
            perror("Could not create socket.");

        int error = connect(this->fd, this->addr, sizeof(struct sockaddr_un));
        if(error < 0)
            perror("Connection error.");
    }

    Client::Details::Details(cstring_t const& ip, uint16_t const& port) :
        buffer(new InputBuffer())
    {
        sockaddr_in* _addr = new sockaddr_in;
        _addr->sin_family = AF_INET;

        _addr->sin_port = htons(port);
        inet_aton(ip, &_addr->sin_addr);

        this->addr = reinterpret_cast<sockaddr*>(_addr);

        this->fd = socket(AF_INET, SOCK_STREAM, 0);
        if(this->fd < 0)
            perror("Could not create socket");

        int error = connect(this->fd, this->addr, sizeof(struct sockaddr_in));
        if(error < 0)
            perror("Connection error");
    }

    Client::Details::~Details()
    {
        shutdown(this->fd, SHUT_WR);
        close(this->fd);

        if(this->addr->sa_family == AF_UNIX)
            delete reinterpret_cast<sockaddr_un*>(this->addr);
        else if(this->addr->sa_family == AF_INET)
            delete reinterpret_cast<sockaddr_in*>(this->addr);

        delete this->buffer;
    }

    ssize_t Client::Details::get_fd() const
    {
        return this->fd;
    }

    void Client::Details::send(cstring_t const& input)
    {
        int result = ::send(this->fd, input.begin(), input.size(), 0);
        if(result < 0)
            perror("Send error");
    }

    void Client::Details::send(vector<char> const& input)
    {
        int result = ::send(this->fd, &input[0], input.size(), 0);
        if(result < 0)
            perror("Send error");
    }

    void Client::Details::send(OPCode::client_code code, cstring_t const& name){
        vector<char> output;
        output.reserve(2 + name.size());

        char header = static_cast<uint8_t>(code);
        output.push_back(header);

        output.push_back(name.size());
        output.insert(output.end(), name.begin(), name.end());

        this->send(output);
    }

    void Client::Details::send(OPCode::client_code code, cstring_t const& name, cstring_t const& payload){
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

    void Client::Details::receive()
    {
        this->buffer->receive(this->fd);
        while(this->can_process_input())
            this->process_input();
    }

    void Client::Details::receive_non_block()
    {
        this->buffer->receive(this->fd, SOCK_NONBLOCK);
        while(this->can_process_input())
            this->process_input();
    }

    void Client::Details::call_counter_listeners
        (cstring_t const& name, uint32_t value)
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
        (cstring_t const& name, cstring_t const& payload)
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
