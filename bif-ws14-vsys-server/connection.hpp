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
#include <unordered_map>
#include <cmath>
#include "common.hpp"
#include "klcp.hpp"
#include <chrono>
#include <thread>
#define BUF 1000
#define FILEBUF 100000

#ifndef connection_connection_h
#define connection_connection_h


#endif

class connection{
private:
    int connection_socket;
    socklen_t addrlen;
    int size;
    struct sockaddr_in address, cliaddress;
public:
    void clearBuffer();
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
    void send_klcp(klcp);
    void send_file(std::string, std::string);
    void getFile(klcp, std::string);
    void close_connection();
    void quit_server();
    std::string lastMessage;
    bool error = true; // true=error, false=ok
    std::string errormsg;
};

void connection::clearBuffer(){
    memset(buffer, NULL, BUF);
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
    request.recieve(&connection_socket);
}

void connection::send_message(std::string message){
    klcp response;
    response.set("type", "message");
    response.set("msg", message);

    response.send(&connection_socket);
}

void connection::send_klcp(klcp _klcp){
    _klcp.send(&connection_socket);
}

void connection::send_command(std::string cmd, std::string val){
    clearBuffer();
    klcp response;
    response.set("type", "command");
    response.set("command", cmd);
    response.set("value", val);
    
    response.send(&connection_socket);
}

void connection::send_file(std::string filename, std::string path){
    clearBuffer();
    
    std::stringstream ss;
    ss << path << "/" << filename;
    std::string file = ss.str();
    ss.clear();
    
    std::ifstream filetosend;
    filetosend.open(file.c_str(), std::ifstream::binary);
    
    if(filetosend){
        
        std::ifstream getfilesize;
        getfilesize.open(file.c_str(),std::ios::ate);
        
        unsigned long filesize = getfilesize.tellg();
        unsigned long blockcount = ceil((float) filesize / FILEBUF);
        unsigned long lastBlockSize = filesize % FILEBUF;
        
        getfilesize.close();
        getfilesize.clear();
        
        
        clearBuffer();
        klcp response;
        response.set("type", "file");
        response.set("filename", filename);
        response.setLong("filesize", filesize);
        response.setLong("blocksize", FILEBUF);
        response.setLong("blockcount", blockcount);
        response.setLong("lastblocksize", lastBlockSize);
        
        response.send(&connection_socket);
        
        char FileBuffer[FILEBUF];
        
        for(unsigned long block = 0; block < blockcount; block++){
            
            unsigned long blocksize = FILEBUF;
            if(block == (blockcount-1)){
                blocksize = lastBlockSize;
            };
            
            float progress = ((float)block / (float)blockcount);
            
            int barWidth = 70;
            
            std::cout << "[";
            int pos = barWidth * progress;
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << round(progress*100.0) << "%\r";
            std::cout.flush();
            
            filetosend.read(FileBuffer, blocksize);
            writen(connection_socket, FileBuffer, blocksize);
        }
        
        filetosend.close();
        
        
        std::cout << std::endl << std::endl;


        send_message("File Send Done!");
        
    }else{
        send_message("File not found");
    }
    
}

void connection::getFile(klcp request2, std::string filepath){

    std::stringstream ss;
    ss << filepath << "/" << request2.get("filename");
    std::string file = ss.str();
    ss.clear();
    
std::ofstream filetosave;
    filetosave.open(file.c_str(), std::ofstream::binary);
    
    unsigned long _blocksize = request.getLong("blocksize");
    unsigned long blockcount = request.getLong("blockcount");
    unsigned long lastBlockSize = request.getLong("lastblocksize");
    
    char FileBuffer[FILEBUF];
    for(unsigned long block = 0; block < blockcount; block++){

        unsigned long blocksize = FILEBUF;
        if(block == (blockcount-1)){
            blocksize = lastBlockSize;
        };
        
        float progress = ((float)block / (float)blockcount);
        
        int barWidth = 70;
        
        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << round(progress*100.0) << "%\r";
        std::cout.flush();
        
        readn(connection_socket, FileBuffer, blocksize);
        filetosave.write(FileBuffer, blocksize);
        
    }
    
    std::cout << std::endl << std::endl;
    
    send_message("Got The File!");
    
}

void connection::close_connection(){
    close(connection_socket);
}

void connection::quit_server(){
    close(connection_socket);
}