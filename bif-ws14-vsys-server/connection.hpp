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
    int connection_socket;
    socklen_t addrlen;
    int size;
    struct sockaddr_in address, cliaddress;
    void clearBuffer();
public:
    klcp request;
    char buffer[BUF];
    connection(int);
    connection();
    void new_server_connection();
    void new_client_connection(int, std::string);
    void recieve_message();
    void recieve_binary();
    void send_message(std::string);
    void send_command(std::string, std::string);
    void send_file(std::string, std::string);
    void getFile(klcp, std::string);
    void close_connection();
    void quit_server();
    std::string lastMessage;
    bool error = true; // true=error, false=ok
    std::string errormsg;
};

void connection::clearBuffer(){
    memset(buffer, 0, BUF);
}

connection::connection(int port){
    connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if(bind(connection_socket, (struct sockaddr *) &address, sizeof(address)) != 0){
        error = true;
        std::cerr << "BIND ERROR - Could not bind to Port " << port << ": " << strerror(errno) << std::endl;
    }else{
        error = false;
        listen(connection_socket, 5);
        
        addrlen = sizeof(struct sockaddr_in);
    }
};

connection::connection(){
    connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(connection_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void connection::new_server_connection(){
    connection_socket = accept(connection_socket, (struct sockaddr *) &cliaddress, &addrlen);
    if(connection_socket > 0){
        std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << std::endl;
    }else{
        std::cout << "whoops";
    }
    clearBuffer();
}

void connection::new_client_connection(int port, std::string _adress){
    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_aton(_adress.c_str(), &address.sin_addr);
    
    if(connect(connection_socket, (struct sockaddr *) &address, sizeof(address)) == 0){
        std::cout << "Success!";
    }else{
        std::cout << "connection failed!";
    }
    clearBuffer();
}

void connection::recieve_message(){
    clearBuffer();
    read(connection_socket, buffer, sizeof(buffer));
    std::string rcvmsg(buffer);
    request.msgParse(rcvmsg);
}

void connection::send_message(std::string message){
    clearBuffer();
    klcp response;
    response.setString("type", "message");
    response.setString("msg", message);
    
    strcpy(buffer, response.msgSerialize().c_str());
    write(connection_socket, buffer, sizeof(buffer));
}

void connection::send_command(std::string cmd, std::string val){
    clearBuffer();
    klcp response;
    response.setString("type", "command");
    response.setString("msg", cmd);
    response.setString("value", val);
    
    strcpy(buffer, response.msgSerialize().c_str());
    write(connection_socket, buffer, sizeof(buffer));
}

void connection::send_file(std::string filename, std::string path){
    clearBuffer();
    
    std::stringstream ss;
    ss << path << "/" << filename;
    std::string file = ss.str();
    ss.clear();
    
    std::ifstream filetosend;
    filetosend.open(file.c_str(), std::ios::binary);
    
    if(filetosend){
        
        std::ifstream getfilesize;
        getfilesize.open(file.c_str(),std::ios::ate);
        
        unsigned long filesize = getfilesize.tellg();
        unsigned int blockcount = ceil((float) filesize / 512);
        
        getfilesize.close();
        getfilesize.clear();
        
        klcp response;
        
        clearBuffer();
        
        response.setString("type", "file");
        response.setString("filename", filename);
        response.setInt("size", (int) filesize);
        response.setInt("blocksize", 512);
        response.setInt("blockcount", blockcount);
        response.setString("msg", "nuthin");
        
        strcpy(buffer, response.msgSerialize().c_str());
        
        std::cout << "SEIZ:" << sizeof(buffer);
        
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
        send_message("File not found");
    }
    
}

void connection::getFile(klcp request2, std::string filepath){
    send_message("OK");
    std::stringstream ss;
    ss << filepath << "/" << request2.getString("filename");
    std::string file = ss.str();
    ss.clear();
    
    std::ofstream filetosave;
    filetosave.open(file.c_str(), std::ofstream::binary);
    
    char filebuffer[request2.getInt("blocksize")];
    for(int i = 0; i < request2.getInt("blockcount"); i++){
        read(connection_socket, filebuffer, sizeof(filebuffer));
        filetosave.write(filebuffer, sizeof(filebuffer));
    }
}

void connection::close_connection(){
    close(connection_socket);
}

void connection::quit_server(){
    close(connection_socket);
}