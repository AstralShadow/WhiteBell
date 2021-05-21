#include <iostream>
#include <signal.h>
#include "include/config.h"
#include "include/Server.h"

using std::endl;
using std::cout;
void signal_callback_handler(int);


Server* server;


int main()
{
    signal(SIGINT, signal_callback_handler);
    signal(SIGTERM, signal_callback_handler);

    string location = config::get_sock_location();
    if(!config::quiet_mode){
        cout << "Starting WhiteBell server at " << location << endl;
    }
    server = new Server(location);
    server->run();

    return 0;
}

void signal_callback_handler(int signum)
{
    if(!config::quiet_mode){
        cout << "Program interrupted with signal " << signum << endl;
    }
    server->stop();
}
