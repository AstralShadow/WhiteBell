#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_set>
#include <queue>
#include <memory>

using std::string;
using std::unordered_set;
using std::queue;
using std::shared_ptr;
using std::weak_ptr;

struct sockaddr;
class Client;

class Server
{
    public:
        Server(string);
        Server(const Server&) = delete;
        void operator = (const Server&) = delete;
        virtual ~Server();

        void run();
        void stop();

    protected:

    private:
        sockaddr* server_address;
        ssize_t server_socket;
        unordered_set<shared_ptr<Client>> connections;
        queue<weak_ptr<Client>> unprocessed;
        bool running;

        sockaddr* generate_server_address(string location);
        ssize_t create_unix_socket();
        void bind_socket();
        void listen();

        void run_main_thread();
        void run_worker_thread();

        void fill_with_fd(fd_set&);
        void wait_for_input(fd_set&);
        ssize_t find_largest_fd();
        void queue_clients_with_input(fd_set&);
        void accept_new_client_if_avaliable(fd_set&);
        void accept_new_client();
        void add_client(ssize_t);
        void remove_client(shared_ptr<Client>);
        void process_clients_input();




        void disconnect_all_clients();

};

#endif // SERVER_H
