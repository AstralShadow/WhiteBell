#include "Client.h"
#include "Client_InputBuffer.h"
#include "Client_DisconnectedException.h"
#include "Server.h"

Client::Client(ssize_t _fd, Server* _server) :
    server(_server),
    fd(_fd),
    buffer(new Client::InputBuffer())
{
    //ctor
}

Client::~Client()
{
    delete this->buffer;
}

ssize_t Client::get_fd() const
{
    return this->fd;
}

void Client::receive_input()
{
    this->buffer->receive(this->fd);
}

void Client::parse_input()
{

}
