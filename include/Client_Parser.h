#ifndef CLIENT_PARSER_H
#define CLIENT_PARSER_H

#include "Client.h"

class Server;
class InputBuffer;

class Client::Parser
{
    public:
        Parser(Client*);
        Parser(const Parser&) = delete;
        void operator = (const Parser&) = delete;
        virtual ~Parser();

        bool can_parse() const;
        void parse();

    protected:

    private:
        Client* client;
        InputBuffer* input;
        Server* server;

};

#endif // CLIENT_PARSER_H
