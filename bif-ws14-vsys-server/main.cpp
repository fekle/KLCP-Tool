/**
* main.hpp
* Copyright (c) 2014 Felix Klein, Aleksandar Lepojic. All rights reserved.
*/

#include <iostream>
#include <vector>
#include <fstream>
#include "common.hpp"
#include "connection.hpp"
#include "server.hpp"
#include "client.hpp"

int main(int argc, char **argv) {

    /**
    * Check if enough arguments
    */
    if (argc < 5) {
        std::stringstream ss;
        ss << "ERROR: not enough Arguments!" << std::endl;
        ss << "Usage: ./vsys-server PORT FILEPATH";
        printError(ss.str());
        ss.clear();
        return EXIT_FAILURE;
    }

    /**
    * Put Arguments into a vector for easier handling
    */
    std::vector<std::string> args(argv + 1, argv + argc);

    /**
    * Get needed Variables and clear the vector
    */
    std::string mode = args[0];
    std::string adress = args[1];
    int port = stoi(args[2]);
    std::string filepath = args[3];
    args.clear();

    /**
    * Create a string using stringstream for printInfo() - this step occurs quite often in the code and will NOT be documentated form now on.
    */
    std::stringstream ss;
    ss << BOLDWHITE << "VSYS SERVER/CLIENT" << std::endl << "\t Mode: " << mode << std::endl << "\t Adress: " << adress << std::endl << "\t Port: " << port << std::endl << "\t Filepath: " << filepath << std::endl << RESET;
    printInfo(ss.str());
    ss.clear();

    /**
    * Determine application mode (client or server)
    */
    if (mode == "server") {
        server(port, filepath);
    } else if (mode == "client") {
        client(adress, port, filepath);
    } else {
        printError("Invalid Mode");
        return EXIT_FAILURE;
    }

    /**
    * Be a nice program, say bye to the user!
    */
    printInfo("Bye, see you next time!");

    return EXIT_SUCCESS;
}
