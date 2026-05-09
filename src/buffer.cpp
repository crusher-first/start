#include "buffer.h"
#include <string>

Buffer::Buffer(int init_size) : m_buffer(init_size), m_read_pos(0), m_write_pos(0) {
}

Buffer::~Buffer() {
}

void Buffer::MakeSpace(int len) {
    if (GetWritableBytes() + GetPrependableBytes() < len) {
        m_buffer.resize(m_write_pos + len);
    } else {
        int readable = GetReadableBytes();
        std::memmove(&m_buffer[0], &m_buffer[m_read_pos], readable);
        m_read_pos = 0;
        m_write_pos = readable;
    }
}

void Buffer::EnsureWritable(int len) {
    if (GetWritableBytes() < len) {
        MakeSpace(len);
    }
}

void Buffer::Append(const char* str, int len) {
    EnsureWritable(len);
    std::memcpy(&m_buffer[m_write_pos], str, len);
    m_write_pos += len;
}

void Buffer::Retrieve(int len) {
    if (len >= GetReadableBytes()) {
        RetrieveAll();
    } else {
        m_read_pos += len;
    }
}

void Buffer::RetrieveUntil(const char* end) {
    m_read_pos = end - &m_buffer[0];
}

void Buffer::RetrieveAll() {
    m_read_pos = 0;
    m_write_pos = 0;
}

std::string Buffer::RetrieveAsString(int len) {
    if (len > GetReadableBytes()) len = GetReadableBytes();
    std::string result(GetReadPtr(), len);
    Retrieve(len);
    return result;
}

std::string Buffer::RetrieveAllAsString() {
    std::string result(GetReadPtr(), GetReadableBytes());
    RetrieveAll();
    return result;
}
