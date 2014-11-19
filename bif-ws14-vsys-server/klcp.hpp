/**
* klcp.hpp
* KLCP - Klein Lepojic Communication Protocol
*
* Created by Aleksandar Lepojic & Felix Klein, 2014
*/

#ifndef bif_ws14_vsys_server_klcp_hpp
#define bif_ws14_vsys_server_klcp_hpp

#include <map>
#include <cstring>

#define BUF 1000

/**
* Class for KLCP
*/
class klcp {
private:
    /**
    * map for storing the headers
    */
    std::map<std::string, std::string> headers;
    char buffer[BUF];
public:
    klcp();

    void set(std::string, std::string);

    void setLong(std::string, unsigned long);

    std::string get(std::string);

    unsigned long getLong(std::string);

    bool recieve(int *);

    bool send(int *);
};

/**
* instantiation, sets version
*/
klcp::klcp() {
    memset(buffer, 0, BUF);
    headers["version"] = "0.0.5";
}

/**
* Function for adding a String Header
*/
void klcp::set(std::string key, std::string val) {
    headers[key] = val;
}

/**
* Function for adding a Long header
*/
void klcp::setLong(std::string key, unsigned long val) {
    headers[key] = std::to_string(val);
}

/**
*  Function for getting a String header
*/
std::string klcp::get(std::string key) {
    return headers[key];
}

/**
* Function for getting a Long header
*/
unsigned long klcp::getLong(std::string key) {
    std::string val = headers[key];
    return std::stoul(val);
}

/**
* Function for recieving from Client/Sever.
* Takes a pointer to a socket as argument
*/
bool klcp::recieve(int *socket) {

    /**
    * Error handling
    */
    ssize_t x = readn(*socket, buffer, BUF);
    if (x <= 0) {
        printError("Failed to read from socket or client/server \n disconnected.");

        return false;
    } else {

        /**
        * Parsing the socket content
        * KLCP is inspired by the HTTP protocol:
        *   key: value
        */
        std::stringstream msg(buffer);
        std::string line;
        std::string linepart;

        /**
        * Read content splitted by newline
        *
        */
        while (std::getline(msg, line, '\n')) {
            std::string key;
            std::string val;
            unsigned int i = 0;
            std::stringstream linestream(line);
            /**
            * Read lines splitted by colon
            */
            while (std::getline(linestream, linepart, ':')) {
                /**
                * Determine if we are in the first or second part of the "key:value" pair, and set variables accordingly
                */
                switch (i) {
                    case 0:
                        key = linepart;
                        break;
                    case 1:
                        val = linepart;
                        break;
                    default:
                        key = "";
                        val = "";
                        break;
                }
                i++;
            }
            /**
            * Store into our headers map
            */
            headers[key] = val;
        }
        return true;
    }
}

/**
* Function for sending a header
*/
bool klcp::send(int *socket) {
    std::stringstream msgstream;
    std::string msg;

    /**
    * write into stringstream - "key:value"
    */
    for (const std::pair<std::string, std::string> header: headers) {
        msgstream << header.first << ":" << header.second << "\n";
    }

    /**
    * convert to string and copy into buffer, write to socket with error handling
    */
    msg = msgstream.str();
    std::copy(msg.begin(), msg.end(), buffer);
    ssize_t x = writen(*socket, buffer, BUF);

    if (x <= 0) {
        printError("Failed to write to socket or client/server\n disconnected.");
        return false;
    } else {
        return true;
    }
}

#endif
