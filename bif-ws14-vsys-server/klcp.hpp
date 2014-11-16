//
//  klcp.h
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#include <map>

#ifndef __bif_ws14_vsys_server__klcp__
#define __bif_ws14_vsys_server__klcp__

#define BUF 1000
#define FILEBUF 100000


class klcp{
private:
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

klcp::klcp(){
    memset(buffer, 0, BUF);
    headers["version"] = "0.0.1";
}

void klcp::set(std::string key, std::string val){
    headers[key] = val;
}

void klcp::setLong(std::string key, unsigned long val){
    headers[key] = std::to_string(val);
}

std::string klcp::get(std::string key){
    return headers[key];
}

unsigned long klcp::getLong(std::string key){
    std::string val = headers[key];
    return std::stoul(val);
}

bool klcp::recieve(int *socket) {

    ssize_t x = readn(*socket, buffer, BUF);

    if (x <= 0) {
        printError("Failed to read from socket or client/server disconnected.");

        return false;
    } else {

        std::stringstream msg(buffer);
        std::string line;
        std::string linepart;

        while (std::getline(msg, line, '\n')) {
            std::string key;
            std::string val;
            unsigned int i = 0;
            std::stringstream linestream(line);
            while (std::getline(linestream, linepart, ':')) {
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
            headers[key] = val;
        }

        return true;
    }
}

bool klcp::send(int *socket) {
    std::stringstream msgstream;
    std::string msg;
    
    for (const std::pair<std::string, std::string> header: headers) {
        msgstream << header.first << ":" << header.second << "\n";
    }
    
    msg = msgstream.str();
    std::copy(msg.begin(), msg.end(), buffer);

    ssize_t x = writen(*socket, buffer, BUF);

    if (x <= 0) {
        printError("Failed to write to socket or client/server disconnected.");

        return false;
    } else {
        return true;
    }
}



#endif /* defined(__bif_ws14_vsys_server__klcp__) */
