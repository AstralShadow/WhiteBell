#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <cstdint>
#include <vector>

using std::vector;

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
        void set_namespace(vector<uint8_t> name);
        void track_event(vector<uint8_t> name);
        void untrack_event(vector<uint8_t> name);
        void trigger_event(vector<uint8_t> name, vector<uint8_t> payload);
        void track_counter(vector<uint8_t> name);
        void untrack_counter(vector<uint8_t> name);
        void join_counter(vector<uint8_t> name);
        void leave_counter(vector<uint8_t> name);

    private:
        class InputBuffer;
        class Parser;

        Server* server;
        ssize_t fd;
        InputBuffer* buffer;
        Parser* parser;


};

#endif // CLIENT_H
