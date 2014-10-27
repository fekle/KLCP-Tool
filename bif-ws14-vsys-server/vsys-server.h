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
#include "request.h"
#include <unordered_map>
#include <cmath>
#define BUF 1024

#ifndef VSYS_Server_vsys_server_h
#define VSYS_Server_vsys_server_h


#endif

std::string filesInDir(std::string path){
    std::string files;
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL){
        files = "Available Files:\n";
        int i = 0;
        while ((ent = readdir (dir)) != NULL){
            if(i>1){
                files += ent->d_name;
                files += " ";
            }
            i++;
        }
        closedir (dir);
    }else{
        files = "Directory not found";
    }
    
    return files;
};


class vsys_server{
private:
    int create_socket, connection;
    socklen_t addrlen;
    int size;
    struct sockaddr_in address, cliaddress;
    void clearBuffer();
public:
    request lastRequest;
    char buffer[BUF];
    vsys_server(int);
    void new_connection();
    void recieve_message();
    void send_message(std::string);
    void send_file(std::string, std::string);
    void close_connection();
    void quit_server();
    bool error = true; // true=error, false=ok
    std::string errormsg;
};

void vsys_server::clearBuffer(){
    memset(buffer,0,sizeof(buffer));
}

vsys_server::vsys_server(int port){
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

void vsys_server::new_connection(){
    connection = accept(create_socket, (struct sockaddr *) &cliaddress, &addrlen);
    if(connection > 0){
        std::cout << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port) << std::endl;
        strcpy(buffer,"Welcome to myserver, Please enter your command:\n");
        send(connection, buffer, strlen(buffer),0);
    }else{
        std::cout << "whoops";
    }
}

void vsys_server::recieve_message(){
    clearBuffer();
    read(connection, buffer, sizeof(buffer));
    lastRequest.init(buffer);
}

void vsys_server::send_message(std::string message){
    clearBuffer();
    
    std::stringstream responseStream;
    std::string responseMessage;
    
    responseStream << "KLCP/0.0.1 " << lastRequest.getStatusCode() << " " << lastRequest.getStatus() << std::endl << "Content-Type: text; charset=utf-8" << std::endl << "Content-Length: " << message.size() << std::endl << std::endl << message;
    
    responseMessage = responseStream.str();
    
    strcpy(buffer, responseMessage.c_str());
    write(connection, buffer, sizeof(buffer));
}

void vsys_server::send_file(std::string filename, std::string path){
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
        write(connection, buffer, sizeof(buffer));
        
        /* Read data from file and send it */
        char filebuffer[512];
        while (filetosend.read(filebuffer, 512)){
            write(connection, filebuffer, filetosend.gcount());
        }
        
        if (filetosend.eof()){
            if (filetosend.gcount() > 0){
                // Still a few bytes left to write
                write(connection, filebuffer, filetosend.gcount());
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
        write(connection, buffer, sizeof(buffer));
    }
    
}

void vsys_server::close_connection(){
    close(connection);
}

void vsys_server::quit_server(){
    close(create_socket);
}