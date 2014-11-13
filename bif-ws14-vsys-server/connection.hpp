//
//  vsys-server.h
//  VSYS-Server
//
//  Created by Felix Klein on 24.10.14.
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
#include "request.hpp"
#include <unordered_map>
#include <cmath>
#include "common.hpp"
#include "klcp.hpp"
#define BUF 1024

#ifndef connection_connection_h
#define connection_connection_h


#endif

class connection{
private:
    int create_socket, connection_socket;
    socklen_t addrlen;
    int size;
    struct sockaddr_in address, cliaddress;
    void clearBuffer();
public:
    klcp request;
    klcp response;
    char buffer[BUF];
    connection(int);
    connection();
    void new_server_connection();
    void new_client_connection(int, std::string);
    void recieve_message();
    void send_message(std::string);
    void send_file(std::string, std::string);
    void close_connection();
    void quit_server();
    bool error = true; // true=error, false=ok
    std::string errormsg;
};

void connection::clearBuffer(){
    memset(buffer,0,sizeof(buffer));
}

connection::connection(int port){
    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if(bind(create_socket, (struct sockaddr *) &address, sizeof(address)) != 0){
        error = true;
        std::cerr << "BIND ERROR - Could not bind to Port " << port << ": " << strerror(errno) << std::endl;
    }else{
        error = false;
        listen(create_socket, 5);
        
        addrlen = sizeof(struct sockaddr_in);
    }
};

connection::connection(){
    create_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(create_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void connection::new_server_connection(){
    connection_socket = accept(create_socket, (struct sockaddr *) &cliaddress, &addrlen);
    if(connection_socket > 0){
        std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << std::endl;
        strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
        send(connection_socket, buffer, strlen(buffer),0);
    }else{
        std::cout << "whoops";
    }
}

void connection::new_client_connection(int port, std::string _adress){
    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons (port);
    inet_aton(_adress.c_str(), &address.sin_addr);
    
    if(connect(connection_socket, (struct sockaddr *) &address, sizeof (address)) == 0){
        // LOL YOLO BANANE
    }
}

void connection::recieve_message(){
    clearBuffer();
    read(connection_socket, buffer, sizeof(buffer));
    std::string rcvmsg(buffer);
    request.msgParse(rcvmsg);
}

void connection::send_message(std::string message){
    clearBuffer();
    
    response.setString("type", "message");
    response.setString("msg", message);
    
    strcpy(buffer, response.msgSerialize().c_str());
    write(connection_socket, buffer, sizeof(buffer));
}

void connection::send_file(std::string filename, std::string path){
    clearBuffer();
    strcpy(buffer, filename.c_str());
    
    
    std::stringstream ss;
    ss << path << "/" << filename;
    std::string file = ss.str();
    ss.clear();
    
    std::ifstream filetosend;
    filetosend.open(file.c_str(),std::ios::binary);
    
    std::stringstream responseStream;
    std::string responseMessage;
    
    if(filetosend){
        
        std::ifstream getfilesize;
        getfilesize.open(file.c_str(),std::ios::ate);
        
        unsigned long filesize = getfilesize.tellg();
        unsigned int blockcount = ceil((float) filesize / 512);
        
        getfilesize.close();
        getfilesize.clear();
        
        responseStream << "KLCP/0.0.1 200 OK" << std::endl << "Content-Type: file" << std::endl << "Content-Length: " << filesize << std::endl << "Block-Size: " << 512 << std::endl << "Block-Count: " << blockcount << std::endl << std::endl;
        
        responseMessage = responseStream.str();
        std::cout << responseMessage;
        clearBuffer();
        strcpy(buffer, responseMessage.c_str());
        write(connection_socket, buffer, sizeof(buffer));
        
        /* Read data from file and send it */
        char filebuffer[512];
        while (filetosend.read(filebuffer, 512)){
            write(connection_socket, filebuffer, filetosend.gcount());
        }
        
        if (filetosend.eof()){
            if (filetosend.gcount() > 0){
                // Still a few bytes left to write
                write(connection_socket, filebuffer, filetosend.gcount());
            }
        }else if (filetosend.bad()){
            // Error reading
        }
        
        filetosend.close();
        filetosend.clear();
        std::cout << "file Send Done";
        
    }else{
        responseStream << "KLCP/0.0.1 404 FILE NOT FOUND" << std::endl << std::endl;
        
        responseMessage = responseStream.str();
        
        clearBuffer();
        strcpy(buffer, responseMessage.c_str());
        write(connection_socket, buffer, sizeof(buffer));
    }
    
}

void connection::close_connection(){
    close(connection_socket);
}

void connection::quit_server(){
    close(create_socket);
}