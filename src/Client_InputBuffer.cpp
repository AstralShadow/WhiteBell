#include "Client.h"
#include "Client_InputBuffer.h"
#include "Client_DisconnectedException.h"
#include <cstdint>
#include <sys/socket.h>
#include <vector>
#include <algorithm>

using std::vector;
using std::copy;
using std::move;

Client::InputBuffer::InputBuffer() :
    size(512),
    buffer(new uint8_t[this->size]),
    read_ptr(this->buffer),
    write_ptr(this->buffer)
{

}

Client::InputBuffer::~InputBuffer()
{
    delete this->buffer;
}

size_t Client::InputBuffer::avaliable()
{
    return this->write_ptr - this->read_ptr;
}

vector<uint8_t> Client::InputBuffer::get()
{
    auto result = this->peek();
    this->read_ptr = this->write_ptr;
    return result;
}

vector<uint8_t> Client::InputBuffer::get(size_t length)
{
    auto result = this->peek(length);
    this->read_ptr += result.size();
    return result;
}

vector<uint8_t> Client::InputBuffer::peek()
{
    return vector<uint8_t>(this->read_ptr, this->write_ptr);
}

vector<uint8_t> Client::InputBuffer::peek(size_t length)
{
    if(length > this->avaliable())
        length = this->avaliable();
    return vector<uint8_t>(this->read_ptr, this->read_ptr + length);
}

size_t Client::InputBuffer::empty_space()
{
    size_t used = this->write_ptr - this->buffer;
    return this->size - used;
}

void Client::InputBuffer::resize(size_t new_size)
{
    uint8_t* new_buffer = new uint8_t[new_size];
    copy(this->read_ptr, this->write_ptr, new_buffer);
    delete this->buffer;

    this->write_ptr = new_buffer + this->avaliable();

    this->size = new_size;
    this->read_ptr = new_buffer;
    this->buffer = new_buffer;
}

void Client::InputBuffer::collect_garbage()
{
    move(this->read_ptr, this->write_ptr, this->buffer);
    this->write_ptr = this->buffer + this->avaliable();
    this->read_ptr = this->buffer;
}

void Client::InputBuffer::receive(ssize_t fd)
{
    this->ensure_some_free_space();
    this->receive_input(fd);
    this->shrink_if_mostly_unused();
}

void Client::InputBuffer::ensure_some_free_space()
{
    size_t garbage = this->read_ptr - this->buffer;
    if(garbage > this->size / 3){
        this->collect_garbage();
    }
    if(this->empty_space() < this->size / 4){
        this->resize(this->size * 2);
    }
}

void Client::InputBuffer::receive_input(ssize_t fd)
{
    ssize_t received = recv(fd, this->write_ptr, this->empty_space(), 0);

    if(received < 0){
        perror("Error while receiving input");
        return;
    }
    if(received == 0){
        throw new Client::DisconnectedException(fd);
    }

    this->write_ptr += received;
}

void Client::InputBuffer::shrink_if_mostly_unused()
{
    if(this->empty_space() > this->size * 3 / 4){
        this->resize(this->size / 2);
    }
}
