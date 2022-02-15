#include "HTTPThread.h"


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



    string body = vTokens[1].c_str();

    string header = "HTTP/1.1 200 OK\r\n";
    header += "Content-Type: text/html\r\n";
    header += "Content-Length: " + to_string(body.size()) + "\r\n";
    header += "\r\n\r\n";

    string response = header + body;

    send(clientSocket, response.c_str(), response.size(), 0);



    closesocket(clientSocket);

}

