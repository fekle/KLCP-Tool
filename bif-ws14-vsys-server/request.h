//
//  request.h
//  VSYS-Server
//
//  Created by Felix Klein on 26.10.14.
//  Copyright (c) 2014 Felix Klein, Aleksandar Lepojic. All rights reserved.
//

#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <dirent.h>
#include <unordered_map>
#define BUF 1024

#ifndef VSYS_Server_request_h
#define VSYS_Server_request_h


#endif

class request{
private:
    std::string type;
    std::string url;
    std::string protocol;
    std::unordered_map<std::string, std::string> headers;
    bool valid = true;
    int statusCode = 500;
public:
    std::string getType();
    std::string getUrl();
    std::string getProtocol();
    std::string getHeader(std::string);
    int getStatusCode();
    std::string getStatus();
    void init(char[BUF]);
};

int request::getStatusCode(){
    return statusCode;
}

std::string request::getStatus(){
    switch (statusCode) {
        case 200:
            return "OK";
            break;
        case 404:
            return "FILE NOT FOUND";
        default:
            return "INTERNAL SERVER ERROR";
            break;
    }
}

std::string request::getType(){
    return type;
}

std::string request::getUrl(){
    return url;
}

std::string request::getProtocol(){
    return protocol;
}

std::string request::getHeader(std::string name){
    return headers[name];
}

void request::init(char msg[BUF]){
    std::stringstream msgstream;
    msgstream << msg;
    std::string line;
    std::string linepart;
    bool firstline = true;
    
    while(std::getline(msgstream, line)) {
        std::stringstream tmps;
        line.erase(line.length()-1);
        tmps << line;
        if(firstline){
            int i = 0;
            while(std::getline(tmps, linepart, ' ')){
                switch (i){
                    case 0:
                        type = linepart;
                        break;
                    case 1:
                        url = linepart;
                        break;
                    case 2:
                        protocol = linepart;
                        break;
                    default:
                        valid = false;
                        break;
                }
                i++;
            }
            firstline = false;
        }else{
            bool isKey = true;
            std::string key;
            while(std::getline(tmps, linepart, ':')){
                if(isKey){
                    if(linepart[0] == ' '){
                        key = linepart.erase(0,1);
                    }else{
                        key = linepart;
                    }
                    isKey = false;
                }else{
                    if(linepart[0] == ' '){
                        headers[key] = linepart.erase(0,1);
                    }else{
                        headers[key] = linepart;
                    }
                    isKey = true;
                }
            }
        }
    }
    
    std::cout << std::endl << "DEBUG: HEADERS ===============================" << std::endl;
    std::cout << "Type: " << type << std::endl;
    std::cout << "Url: " << url << std::endl;
    std::cout << "Protocol: " << protocol << std::endl;
    for(std::unordered_map<std::string, std::string>::iterator iter = headers.begin(); iter != headers.end(); ++iter){
        std::cout << iter->first << " ---> " << iter->second << ";" << std::endl;
    }
    std::cout << "==============================================" << std::endl << std::endl;
    
    statusCode = 200;
}
