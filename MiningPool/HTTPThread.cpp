#include "HTTPThread.h"


inline bool ends_with(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void HTTPThread::clientWorker(int clientSocket, Global* global) {

    const unsigned int MAX_BUF_LENGTH = 4096;
    std::vector<char> buffer(MAX_BUF_LENGTH);
    std::string httpRequest;
    int bytesReceived = 0;
    do {
        bytesReceived = recv(clientSocket, &buffer[0], buffer.size(), 0);
        if (bytesReceived == -1) {
            return;
        }
        else {
            httpRequest.append(buffer.cbegin(), buffer.cend());
        }
    } while (bytesReceived == MAX_BUF_LENGTH);


    string URI;
    istringstream sReq(httpRequest);
    getline(sReq, URI);

    istringstream sTokens(URI);
    vector<string> vTokens((istream_iterator<string>(sTokens)),  istream_iterator<string>());

    string URL = vTokens[1];

    char* content;
    int contentLen = 0;
    if (global->webpack->pages.count(URL) == 0) {
        closesocket(clientSocket);
        return;
    }
    else {
        content = global->webpack->pages[URL].data;
        contentLen = global->webpack->pages[URL].len;
    }

    string mimeType = "text/html";
    if (ends_with(URL, ".css"))
        mimeType = "text/css";
    else if (ends_with(URL, ".png"))
        mimeType = "image/png";
    else if (ends_with(URL, ".js"))
        mimeType = "text/javascript";


    string header = "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: " + mimeType + "\r\n";
    header += "Content-Length: " + to_string(contentLen) + "\r\n";
    header += "\r\n";

    send(clientSocket, header.c_str(), header.size(), 0);
    send(clientSocket, content, contentLen, 0);

    closesocket(clientSocket);

}

