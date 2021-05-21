#include "Client.h"
#include "Client_InputBuffer.h"
#include "Client_DisconnectedException.h"
#include "Client_Parser.h"
#include "Server.h"

Client::Client(ssize_t _fd, Server* _server) :
    server(_server),
    fd(_fd),
    buffer(new Client::InputBuffer()),
    parser(new Client::Parser(this))
{
    //ctor
}

Client::~Client()
{
    delete this->buffer;
    delete this->parser;
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
    while(this->parser->can_parse()){
        this->parser->parse();
    }
}
