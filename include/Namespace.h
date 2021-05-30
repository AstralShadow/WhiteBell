#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <cstdint>
#include <vector>
#include <memory>
#include <map>

class Client;
using std::vector;
using std::shared_ptr;
using std::weak_ptr;
using std::map;

typedef vector<uint8_t> name_t;
typedef vector<uint8_t> payload_t;
typedef vector<weak_ptr<Client>> client_list;

class Namespace
{
    public:
        Namespace(name_t);
        virtual ~Namespace() = default;

        name_t get_name() const;

        void add_client(shared_ptr<Client>);
        void remove_client(shared_ptr<Client>);

        void add_event_listener(const name_t&, shared_ptr<Client>);
        void remove_event_listener(const name_t&, shared_ptr<Client>);
        void add_counter_listener(const name_t&, shared_ptr<Client>);
        void remove_counter_listener(const name_t&, shared_ptr<Client>);
        void join_counter(const name_t&, shared_ptr<Client>);
        void leave_counter(const name_t&, shared_ptr<Client>);

        void trigger_event(const name_t&, const payload_t&);

    protected:

    private:
        name_t _name;

        client_list clients = {};
        map<name_t, client_list> event_listeners = {};
        map<name_t, client_list> counter_listeners = {};
        map<name_t, client_list> counters = {};

        void trigger_counter_event(const name_t&);
};

#endif // NAMESPACE_H
