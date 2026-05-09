#include "http.h"
#include <cstring>
#include <regex>

HttpRequest::HttpRequest() : m_state(PARSE_REQUEST_LINE), m_method(METHOD_GET) {
}

HttpRequest::~HttpRequest() {
}

void HttpRequest::Reset() {
    m_state = PARSE_REQUEST_LINE;
    m_method = METHOD_GET;
    m_path.clear();
    m_version.clear();
    m_body.clear();
    m_headers.clear();
}

std::string HttpRequest::GetHeader(const std::string& key) const {
    auto it = m_headers.find(key);
    if (it != m_headers.end()) {
        return it->second;
    }
    return "";
}

bool HttpRequest::IsKeepAlive() const {
    auto it = m_headers.find("Connection");
    if (it != m_headers.end()) {
        return it->second == "keep-alive" || it->second == "Keep-Alive";
    }
    return false;
}

HttpRequest::HTTP_CODE HttpRequest::ParseRequestLine(const char* line) {
    std::regex pattern("^([^ ]+) ([^ ]+) ([^ ]+)\r\n$");
    std::cmatch result;
    if (std::regex_match(line, result, pattern)) {
        std::string method = result[1];
        if (method == "GET") m_method = METHOD_GET;
        else if (method == "POST") m_method = METHOD_POST;
        else if (method == "HEAD") m_method = METHOD_HEAD;
        else return BAD_REQUEST;

        m_path = result[2];
        m_version = result[3];
        m_state = PARSE_HEADERS;
        return NO_REQUEST;
    }
    return BAD_REQUEST;
}

HttpRequest::HTTP_CODE HttpRequest::ParseHeaders(const char* line) {
    if (strlen(line) == 0) {
        if (m_method == METHOD_GET) {
            m_state = PARSE_DONE;
            return GET_REQUEST;
        }
        m_state = PARSE_BODY;
        return NO_REQUEST;
    }

    std::regex pattern("^([^:]+): (.*)\r\n$");
    std::cmatch result;
    if (std::regex_match(line, result, pattern)) {
        m_headers[result[1]] = result[2];
    }
    return NO_REQUEST;
}

HttpRequest::HTTP_CODE HttpRequest::ParseBody(const char* line) {
    m_body = line;
    m_state = PARSE_DONE;
    return POST_REQUEST;
}

HttpRequest::HTTP_CODE HttpRequest::Parse(char* buffer, int len) {
    if (len <= 0) return NO_REQUEST;
    
    char* p = buffer;
    char* end = buffer + len;

    while (p < end && m_state != PARSE_DONE) {
        char* line_end = (char*)memchr(p, '\n', end - p);
        if (!line_end) break;

        *line_end = '\0';
        if (line_end > p && *(line_end - 1) == '\r') *(line_end - 1) = '\0';

        HTTP_CODE ret = NO_REQUEST;
        switch (m_state) {
            case PARSE_REQUEST_LINE:
                ret = ParseRequestLine(p);
                break;
            case PARSE_HEADERS:
                ret = ParseHeaders(p);
                break;
            case PARSE_BODY:
                ret = ParseBody(p);
                break;
            default:
                break;
        }

        if (ret != NO_REQUEST) return ret;
        p = line_end + 1;
    }

    return NO_REQUEST;
}
