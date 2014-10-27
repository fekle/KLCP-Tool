//
//  main.cpp
//  VSYS-Server
//
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
#include <string.h>
#include <vector>
#include <fstream>
#include <dirent.h>
#include "vsys-server.h"
#define BUF 1024



int main(int argc, char **argv){
    
    if(argc<3){
        std::cout << "ERROR: noth enough Argumetns!" << std::endl;
        std::cout << "Usage: ./vsys-server PORT FILEPATH" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::vector<std::string> args(argv + 1, argv + argc);
    
    int port = stoi(args[0]);
    std::string filepath = args[1];
    
    std::cout << "VSYS SERVER" << std::endl << "\t Port: " << port << std::endl << "\t Filepath: " << filepath << std::endl << "===" << std::endl << std::endl;
    
    vsys_server server(port);
    
    while(1){
        std::cout << "Waiting for connections..." << std::endl;
        server.new_connection();
        
        do{
            std::cout << std::endl << "Wating for message... " << std::endl;
            server.recieve_message();
            std::string requestType = server.lastRequest.getType();
            
            if(requestType == "LIST"){
                server.send_message(filesInDir(filepath));
            }else if(requestType == "GET"){
                server.send_file(server.lastRequest.getUrl(), filepath);
            }else if(strncmp(server.buffer, "QUIT", 4)){
                server.send_message("Invalid Command");
            }
            
            
        }while(strncmp(server.buffer, "QUIT", 4)  != 0);
        
        server.close_connection();
        
        if(strncmp(server.buffer, "quitall", 7)  == 0){
            std::cout << "Client told me to quit, I obey." << std::endl;
            break;
        }
    }
    
    server.quit_server();
    return EXIT_SUCCESS;
}