//
//  main.cpp
//  VSYS-Server
//
//  Copyright (c) 2014 Felix Klein, Aleksandar Lepojic. All rights reserved.
//

#include <iostream>
#include <vector>
#include <fstream>
#include "common.hpp"
#include "connection.hpp"
#include "server.hpp"
#include "client.hpp"

int main(int argc, char **argv){
    
    if(argc<5){
        std::stringstream ss;
        ss << "ERROR: not enough Arguments!" << std::endl;
        ss << "Usage: ./vsys-server PORT FILEPATH";
        printError(ss.str());
        ss.clear();
        return EXIT_FAILURE;
    }
    
    std::vector<std::string> args(argv + 1, argv + argc);
    
    std::string type = args[0];
    
//    type = "client";
    
    std::string adress = args[1];
    int port = stoi(args[2]);
    std::string filepath = args[3];

    std::stringstream ss;

    ss << BOLDWHITE << "VSYS SERVER/CLIENT" << std::endl << "\t Mode: " << type << std::endl << "\t Adress: " << adress << std::endl << "\t Port: " << port << std::endl << "\t Filepath: " << filepath << std::endl << RESET;

    printInfo(ss.str());

    ss.clear();
    
    if(type == "server"){
        server(port, filepath);
    }else if(type == "client"){
        client(adress, port, filepath);
    }else{
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}