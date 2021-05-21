#ifndef CLIENT_DISCONNECTEDEXCEPTION_H
#define CLIENT_DISCONNECTEDEXCEPTION_H

#include "Client.h"
#include <string>
#include <exception>

using std::string;

class Client::DisconnectedException : std::exception
{
    public:
        DisconnectedException(ssize_t client_fd);
        virtual ~DisconnectedException() = default;
        virtual string what();
        ssize_t get_fd();

    protected:

    private:
        ssize_t client_fd;
};

#endif // CLIENT_DISCONNECTEDEXCEPTION_H
