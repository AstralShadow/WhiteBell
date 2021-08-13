# What is WhiteBell
A fast message repeater for simple multiprocess communication.  
Uses POSIX sockets to receive and distribute events.  

## Events
Events are named packages of raw binary data.  
Any application can listen for or dispatch events within.  

## Counters
They give you an easy way to count active connections.  
Each connected client can ask to be counted in given counter 
or check how many clients are in given counter.  

## Namespaces
Think of a namespace as isolated scope for events and counters.  
Their purpose is to keep different applications from colliding.  

## Planned functionality
* Runtime configuration:
	* Permissions, in the form of users.
	* Being able to move namespaces to different socket endpoints
	* Multithreading
	* Portability to different operating systems

# How to install?
For now, you have to compile it yourself with Code::Blocks on
system that supports POSIX sockets.  
The project file is /WhiteBell.cbp

# Configuration
Currently you may need to edit:
* main.cpp - to switch between TCP or UNIX socket mode.
* src/config.cpp - to change your port or UNIX socket location.  
You can configure it as systemd service, based on the example
configuration in examples/whitebell.service

# How to use it?
You need to compile and start the server application.  
You can find implementation of client libraries in clients/  
Currently there are versions for C++, php and ESP8266 (Arduino)  
They share the same interface, with a slight differences.  
You can check it in the wiki or browse the method names in the code.  
_Wiki will be added soon._

# Bugs and troubleshooting
Probably. If you find one, please open an issue.

# License
Licensed under MIT license, as stated in /LICENSE and on github.  
_This is my first open source project, and i will be happy if
you provide me feedback and/or popularize the software._
