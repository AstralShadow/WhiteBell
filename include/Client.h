#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <cstdint>

class Server;

class Client
{

    public:
        class DisconnectedException;

        Client(ssize_t fd, Server*);
        Client(const Client&) = delete;
        void operator = (const Client&) = delete;
        virtual ~Client();

        ssize_t get_fd() const;

        void receive_input();
        void parse_input();

    protected:

    private:
        class InputBuffer;
        class Parser;

        Server* server;
        ssize_t fd;
        InputBuffer* buffer;
        Parser* parser;


};

#endif // CLIENT_H
