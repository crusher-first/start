#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cstring>
#include <algorithm>
#include <string>

class Buffer {
public:
    Buffer(int init_size = 1024);
    ~Buffer();

    int GetReadableBytes() const { return m_write_pos - m_read_pos; }
    int GetWritableBytes() const { return m_buffer.size() - m_write_pos; }
    int GetPrependableBytes() const { return m_read_pos; }

    const char* GetReadPtr() const { return &m_buffer[m_read_pos]; }
    const char* GetWritePtr() const { return &m_buffer[m_write_pos]; }

    void EnsureWritable(int len);
    void Append(const char* str, int len);
    void Append(const std::string& str) { Append(str.c_str(), str.size()); }

    void Retrieve(int len);
    void RetrieveUntil(const char* end);
    void RetrieveAll();

    std::string RetrieveAsString(int len);
    std::string RetrieveAllAsString();

private:
    std::vector<char> m_buffer;
    int m_read_pos;
    int m_write_pos;

    void MakeSpace(int len);
};

#endif // BUFFER_H
