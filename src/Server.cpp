#include "Server.h"
#include "Client.h"
#include "Client_DisconnectedException.h"
#include "Namespace.h"
#include "config.h"
#include <iostream>
#include <string>
#include <unordered_set>
#include <queue>
#include <memory>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

using std::cout;
using std::endl;
using std::string;
using std::unordered_set;
using std::queue;
using std::shared_ptr;
using std::weak_ptr;


const uint8_t Server::version = 102;

Server::Server(string location) :
    server_address(generate_server_address(location)),
    server_addr_len(sizeof(struct sockaddr_un)),
    server_socket(create_unix_socket())
{
    this->bind_socket();
    this->listen();
}

Server::Server(uint16_t port) :
    server_address(generate_server_address(port)),
    server_addr_len(sizeof(struct sockaddr_in)),
    server_socket(create_tcp_socket())
{
    this->bind_socket();
    this->listen();
}

sockaddr* Server::generate_server_address(string location)
{
    sockaddr_un* address = new sockaddr_un;
    address->sun_family = AF_UNIX;
    strcpy(address->sun_path, location.c_str());
    return reinterpret_cast<sockaddr*>(address);
}

sockaddr* Server::generate_server_address(uint16_t port)
{
    sockaddr_in* address = new sockaddr_in;
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    return reinterpret_cast<sockaddr*>(address);
}

ssize_t Server::create_unix_socket()
{
    ssize_t fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0){
        perror("Could not create socket.");
        exit(1);
    }
    return fd;
}

ssize_t Server::create_tcp_socket()
{
    ssize_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        perror("Could not create socket.");
        exit(1);
    }
    return fd;
}

void Server::bind_socket()
{
    ssize_t source_fd = this->server_socket;
    if(bind(source_fd, this->server_address, this->server_addr_len)){
        perror("Binding error.");
        exit(1);
    }
}

void Server::listen()
{
    ssize_t source_fd = this->server_socket;
    if(::listen(source_fd, config::max_pending_connections)){
        perror("Listen error.");
        exit(1);
    }
}

Server::~Server()
{
    this->disconnect_all_clients();
    ::shutdown(this->server_socket, SHUT_WR);
    close(this->server_socket);

    if(this->server_address->sa_family == AF_UNIX)
        this->remove_unix_endpoint();

    delete this->server_address;
}

void Server::remove_unix_endpoint(){
    sockaddr_un* address = reinterpret_cast<sockaddr_un*>(this->server_address);
    if(!config::quiet_mode){
        cout << "Removing: " << address->sun_path << endl;
    }
    remove(address->sun_path);
}

void Server::disconnect_all_clients()
{
    connections.clear();
}

void Server::run()
{
    if(this->running){
        //this->run_worker_thread();
        // TODO implement multithreading.
        return;
    }

    this->run_main_thread();
}

void Server::stop()
{
    this->running = false;
}

void Server::run_main_thread()
{
    fd_set connection_set;
    this->running = true;

    while(running){
        fill_with_fd(connection_set);
        wait_for_input(connection_set);
        queue_clients_with_input(connection_set);
        accept_new_client_if_avaliable(connection_set);
        process_clients_input();
    }
}

void Server::fill_with_fd(fd_set& target)
{
    FD_ZERO(&target);
    FD_SET(this->server_socket, &target);

    auto &clients = this->connections;
    for(auto itr = clients.begin(); itr != clients.end(); ++itr){
        Client* client = itr->get();
        FD_SET(client->get_fd(), &target);
    }
}

void Server::wait_for_input(fd_set& connection_set)
{
    int largest_fd = this->find_largest_fd();
    int limit = largest_fd + 1;
    if(select(limit, &connection_set, nullptr, nullptr, NULL) < 0){
        perror("Select error.");
    }
}

ssize_t Server::find_largest_fd()
{
    ssize_t largest = this->server_socket;
    for(auto itr = connections.begin(); itr != connections.end(); ++itr){
        Client* client = itr->get();
        if(largest < client->get_fd()){
            largest = client->get_fd();
        }
    }
    return largest;
}

void Server::queue_clients_with_input(fd_set& connection_set)
{
    auto &clients = this->connections;

    for(auto itr = clients.begin(); itr != clients.end(); ++itr){
        Client* client = itr->get();
        ssize_t fd = client->get_fd();
        if(FD_ISSET(fd, &connection_set)){
            FD_CLR(fd, &connection_set);
            this->unprocessed.push(weak_ptr<Client>(*itr));
        }
    }
}

void Server::accept_new_client_if_avaliable(fd_set& connection_set)
{
    ssize_t server_fd = this->server_socket;
    if(FD_ISSET(server_fd, &connection_set)){
        FD_CLR(server_fd, &connection_set);
        this->accept_new_client();
    }
}

void Server::accept_new_client()
{
    ssize_t server_fd = this->server_socket;
    sockaddr* address = this->server_address;
    socklen_t address_size = this->server_addr_len;

    ssize_t client_fd = accept4(server_fd, address, &address_size, SOCK_NONBLOCK);

    if(client_fd < 0){
        perror("Error while accepting client.");
        return;
    }

    this->add_client(client_fd);
}

void Server::add_client(ssize_t client_fd)
{
    Client* client = new Client(client_fd, this);
    this->connections.insert(client->get_ptr());
}

void Server::remove_client(shared_ptr<Client> client)
{
    shared_ptr<Namespace> space = client->get_namespace();
    if(space != nullptr){
        space->remove_client(client);
    }

    auto itr = this->connections.find(client);
    this->connections.erase(itr);
}

void Server::process_clients_input()
{
    while(this->unprocessed.size() > 0){
        weak_ptr<Client> wp = this->unprocessed.front();
        shared_ptr<Client> sp = wp.lock();
        this->unprocessed.pop();
        if(sp){
            Client* client = sp.get();
            try{
                client->receive_input();
                client->parse_input();
            }catch(Client::DisconnectedException&){
                this->remove_client(sp);
            }
        }
    }
}

shared_ptr<Namespace> Server::get_namespace(name_t name)
{
    auto itr = this->namespaces.begin();
    while(itr != this->namespaces.end()){
        shared_ptr<Namespace> ptr = itr->lock();
        if(!ptr){
            itr = this->namespaces.erase(itr);
            continue;
        }

        if(ptr->get_name() == name)
            return ptr;

        ++itr;
    }

    shared_ptr<Namespace> ptr(new Namespace(name));
    this->namespaces.push_back(weak_ptr<Namespace>(ptr));
    return ptr;
}

