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
#include "connection.hpp"
#include "server.hpp"
#include "client.hpp"
#define BUF 1024



int main(int argc, char **argv){
    
    if(argc<3){
        std::cout << "ERROR: noth enough Argumetns!" << std::endl;
        std::cout << "Usage: ./vsys-server PORT FILEPATH" << std::endl;
        return EXIT_FAILURE;
    }
    
    std::vector<std::string> args(argv + 1, argv + argc);
    
    std::string type = args[0];
    std::string adress = args[1];
    int port = stoi(args[2]);
    std::string filepath = args[3];
    
    std::cout << "VSYS SERVER" << std::endl << "\t Port: " << port << std::endl << "\t Filepath: " << filepath << std::endl << "===" << std::endl << std::endl;
    
    if(type == "server"){
        server(port, filepath);
    }else if(type == "client"){
        client(adress, port, filepath);
    }else{
        return 1;
    }
    
    return EXIT_SUCCESS;
}