#include "Client.h"
#include "Client_DisconnectedException.h"
#include <string>

using std::to_string;

Client::DisconnectedException::DisconnectedException(ssize_t fd) :
    client_fd(fd)
{
    // ctor
}

string Client::DisconnectedException::what()
{
    return "Client " + to_string(this->client_fd) + " Disconnected.";
}

ssize_t Client::DisconnectedException::get_fd()
{
    return this->client_fd;
}

