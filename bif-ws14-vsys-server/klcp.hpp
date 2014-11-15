//
//  klcp.h
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#ifndef __bif_ws14_vsys_server__klcp__
#define __bif_ws14_vsys_server__klcp__

#include <stdio.h>
#include <string>
#include <map>
#include <strstream>
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
    
    void recieve(int*);
    void send(int*);
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
    return std::stoi(val);
}

void klcp::recieve(int* socket){
    std::vector<std::string> lines;
    read(*socket, buffer, BUF);
    std::cout << std::endl << "Recieving: " << std::endl << buffer << std::endl;
    std::stringstream msg(buffer);
    std::stringstream msg2;
    std::string line;
    std::string linepart;
    
    while(std::getline(msg, line, '\n')){
        std::string key;
        std::string val;
        unsigned int i = 0;
        std::stringstream linestream(line);
        while(std::getline(linestream, linepart, ':')){
            switch(i){
                case 0:
                    key = linepart;
                    break;
                case 1:
                    val = linepart;
                    break;
            }
            i++;
        }
        headers[key] = val;
    }
}

void klcp::send(int* socket){
    std::stringstream msgstream;
    std::string msg;
    
    for (const std::pair<std::string, std::string> header: headers) {
        msgstream << header.first << ":" << header.second << "\n";
    }
    
    msg = msgstream.str();
    std::copy(msg.begin(), msg.end(), buffer);
    
    std::cout << std::endl << "Sending: " << std::endl << buffer << std::endl;
    
    write(*socket, buffer, BUF);
}



#endif /* defined(__bif_ws14_vsys_server__klcp__) */
