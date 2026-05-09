#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <unordered_map>

class HttpRequest {
public:
    enum Method { METHOD_GET, METHOD_POST, METHOD_HEAD, METHOD_PUT, METHOD_DELETE };
    enum ParseState { PARSE_REQUEST_LINE, PARSE_HEADERS, PARSE_BODY, PARSE_DONE };
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, POST_REQUEST, BAD_REQUEST, NO_RESOURCES, 
                     FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };

    HttpRequest();
    ~HttpRequest();

    void Reset();
    HTTP_CODE Parse(char* buffer, int len);

    Method GetMethod() const { return m_method; }
    std::string GetPath() const { return m_path; }
    std::string GetVersion() const { return m_version; }
    std::string GetHeader(const std::string& key) const;
    bool IsKeepAlive() const;

private:
    HTTP_CODE ParseRequestLine(const char* line);
    HTTP_CODE ParseHeaders(const char* line);
    HTTP_CODE ParseBody(const char* line);

    ParseState m_state;
    Method m_method;
    std::string m_path;
    std::string m_version;
    std::string m_body;
    std::unordered_map<std::string, std::string> m_headers;
};

#endif // HTTP_H
