#include "../include/InputBuffer.hpp"
#include "../include/DisconnectedException.hpp"
#include <cstdint>
#include <sys/socket.h>
#include <vector>
#include <algorithm>

using std::vector;
using std::copy;
using std::move;

namespace WhiteBell
{
    InputBuffer::InputBuffer()
    {
        this->buffer = new uint8_t[this->size];
        this->read_ptr = this->buffer;
        this->write_ptr = this->buffer;
    }

    InputBuffer::~InputBuffer()
    {
        delete this->buffer;
    }

    size_t InputBuffer::avaliable()
    {
        return this->write_ptr - this->read_ptr;
    }

    vector<uint8_t> InputBuffer::get()
    {
        auto result = this->peek();
        this->read_ptr = this->write_ptr;
        return result;
    }

    vector<uint8_t> InputBuffer::get(size_t length)
    {
        auto result = this->peek(length);
        this->read_ptr += result.size();
        return result;
    }

    vector<uint8_t> InputBuffer::peek()
    {
        return vector<uint8_t>(this->read_ptr, this->write_ptr);
    }

    vector<uint8_t> InputBuffer::peek(size_t length)
    {
        if(length > this->avaliable())
            length = this->avaliable();
        return vector<uint8_t>(this->read_ptr, this->read_ptr + length);
    }

    size_t InputBuffer::empty_space()
    {
        size_t used = this->write_ptr - this->buffer;
        return this->size - used;
    }

    void InputBuffer::resize(size_t new_size)
    {
        uint8_t* new_buffer = new uint8_t[new_size];
        copy(this->read_ptr, this->write_ptr, new_buffer);
        delete this->buffer;

        this->write_ptr = new_buffer + this->avaliable();

        this->size = new_size;
        this->read_ptr = new_buffer;
        this->buffer = new_buffer;
    }

    void InputBuffer::collect_garbage()
    {
        move(this->read_ptr, this->write_ptr, this->buffer);
        this->write_ptr = this->buffer + this->avaliable();
        this->read_ptr = this->buffer;
    }

    void InputBuffer::receive(ssize_t fd, uint32_t flags)
    {
        this->ensure_some_free_space();
        this->receive_input(fd, flags);
        this->shrink_if_mostly_unused();
    }

    void InputBuffer::ensure_some_free_space()
    {
        size_t garbage = this->read_ptr - this->buffer;
        if(garbage > this->size / 3){
            this->collect_garbage();
        }
        if(this->empty_space() < this->size / 4){
            this->resize(this->size * 2);
        }
    }

    void InputBuffer::receive_input(ssize_t fd, uint32_t flags)
    {
        ssize_t received = recv(fd, this->write_ptr, this->empty_space(), flags);

        if(received < 0){
            perror("Error while receiving input");
            return;
        }
        if(received == 0){
            throw DisconnectedException();
        }

        this->write_ptr += received;
    }

    void InputBuffer::shrink_if_mostly_unused()
    {
        if(this->empty_space() > this->size * 3 / 4){
            this->resize(this->size / 2);
        }
    }
}
