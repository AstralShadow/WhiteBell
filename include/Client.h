#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <cstdint>
#include <vector>
#include <memory>

using std::vector;
using std::shared_ptr;
using std::weak_ptr;
typedef vector<uint8_t> name_t;
typedef vector<uint8_t> payload_t;

class Server;
class Namespace;

class Client
{
    public:
        class DisconnectedException;

        Client(ssize_t fd, Server*);
        Client(const Client&) = delete;
        void operator = (const Client&) = delete;
        virtual ~Client();

        static const uint8_t protocol_version;

        ssize_t get_fd() const;
        shared_ptr<Client> get_ptr();
        shared_ptr<Namespace> get_namespace() const;

        vector<name_t> get_tracked_events() const;
        vector<name_t> get_tracked_counters() const;
        vector<name_t> get_joined_counters() const;

        void receive_input();
        void parse_input();

        void send_event(const name_t&, const payload_t&);
        void send_counter_event(const name_t&, const uint32_t&);
        void send_counter_event(const name_t&, const uint8_t[4]);

    protected:
        void set_namespace(name_t);
        void track_event(name_t);
        void untrack_event(name_t);
        void trigger_event(name_t, payload_t);
        void track_counter(name_t);
        void untrack_counter(name_t);
        void join_counter(name_t);
        void leave_counter(name_t);

    private:
        class InputBuffer;
        class Parser;

        weak_ptr<Client>* _self = nullptr;

        ssize_t fd;
        Server* server;
        InputBuffer* buffer = nullptr;
        Parser* parser = nullptr;

        shared_ptr<Namespace> _namespace = nullptr;
        vector<name_t> tracked_events;
        vector<name_t> tracked_counters;
        vector<name_t> joined_counters;

        enum struct OutputOPCode{
            version = 0,
            event_triggered = 1,
            counter_changed = 2
        };

        void send(vector<uint8_t>);
        void send_protocol_version();
};

#endif // CLIENT_H
