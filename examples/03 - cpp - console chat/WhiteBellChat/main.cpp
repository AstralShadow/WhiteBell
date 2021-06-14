#include <iostream>
#include <thread>
#include <vector>
#include "WhiteBellClient.hpp"

using namespace std;
using WBClient = WhiteBell::Client;
using std::thread;
using std::vector;


string name = "";

void print_message(WhiteBell::cstring_t _input)
{
    string input = _input;
    uint8_t name_length = _input[0];
    string _name(input.begin() + 1, input.begin() + 1 + name_length);
    string message(input.begin() + 1 + name_length, input.end());
    if(_name != name){
        cout << name << " > " << message << endl;
    }
}

void print_connections_count(uint32_t connections)
{
    cout << "# Online: " << connections << endl;
}

int main()
{;
    WBClient client("127.0.0.1", 25250);
    client.set_namespace("03.Chat");
    cout << "Connected!" << endl;

    while(name.size() > 255 || name.size() == 0){
        cout << "Your name [1-256 symbols]?" << endl << "> ";
        cin >> name;
    }

    client.join_counter("online");
    client.track_event("message", &print_message);
    client.track_counter("online", &print_connections_count);

    // Comment this line and uncomment the one
    //  in while(true){...} to disable async mode.
    thread reader(&WBClient::run, &client);

    while(true){
        // client.fetch();

        string line;
        std::getline(cin, line);

        if(line != "")
            client.trigger_event("message", (((char)name.size()) + name + line).c_str());

    }

    return 0;
}
