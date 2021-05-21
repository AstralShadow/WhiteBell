#include "Client.h"
#include "Client_Parser.h"
#include "Client_InputBuffer.h"
#include "Server.h"

Client::Parser::Parser(Client* _client) :
    client(_client),
    input(_client->buffer),
    server(_client->server)
{
    //ctor
}

Client::Parser::~Parser()
{
    //dtor
}

bool Client::Parser::can_parse() const
{

}

void Client::Parser::parse()
{

}
