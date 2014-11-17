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
#include <thread>
#include "klcp.hpp"
#include "common.hpp"
#include <cstring>

#define BUF 1000
#define FILEBUF 100000

#ifndef connection_connection_h
#define connection_connection_h


#endif

class connection {
private:
    int connection_socket;
public:
    bool skip = false;

    void clearBuffer();

    char buffer[BUF];

    connection(int);

    connection();

    void new_client_connection(int, std::string);

    klcp recieve();

    void send_message(std::string);

    void send_command(std::string);

    void send_command(std::string, std::string);

    void send_login(std::string, std::string);

    void send_file(std::string, std::string);

    void getFile(klcp, std::string);

    void close_connection();

    bool error = false; // true=error, false=ok
    std::string errormsg;
};

connection::connection() {
    clearBuffer();
}

void connection::clearBuffer() {
//    memset(buffer, 0, BUF);
    bzero(buffer, BUF);
}

connection::connection(int socket) {
    connection_socket = socket;
}

void connection::new_client_connection(int port, std::string _adress) {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_aton(_adress.c_str(), &address.sin_addr);

    if (connect(connection_socket, (struct sockaddr *) &address, sizeof(address)) == 0) {
        error = false;
        printInfo("Connected!");
    } else {
        error = true;
        printError("Connection Failed!");
    }
    clearBuffer();
}

klcp connection::recieve() {
    klcp request;

    if (request.recieve(&connection_socket)) {

        if (request.get("type") == "message") {
            unsigned long msglength = (unsigned long) atol(request.get("length").c_str());
            char msgBuffer[msglength];

            readn(connection_socket, msgBuffer, (size_t) msglength);

            std::string msg(msgBuffer);
            msg.resize(msglength, ' ');
            request.set("msg", msg);
        }

        error = false;
    } else {
        error = true;
        errormsg = "recieve failed";
    }

    return request;
}

void connection::send_message(std::string message) {
    unsigned long msglength = message.size();
    char msgBuffer[msglength];
    std::copy(message.begin(), message.end(), msgBuffer);
    klcp response;
    response.set("type", "message");
    response.set("length", std::to_string(msglength));

    if (response.send(&connection_socket)) {
        writen(connection_socket, msgBuffer, msglength);
    } else {
        error = true;
        errormsg = "send failed";
    }
}


void connection::send_command(std::string cmd, std::string val) {
    clearBuffer();
    klcp response;
    response.set("type", "command");
    response.set("command", cmd);
    response.set("value", val);

    if (response.send(&connection_socket)) {
        error = false;
    } else {
        error = true;
        errormsg = "send failed";
    }
}

void connection::send_command(std::string cmd) {
    clearBuffer();
    klcp response;
    response.set("type", "command");
    response.set("command", cmd);
    response.set("value", "");

    if (response.send(&connection_socket)) {
        error = false;
    } else {
        error = true;
        errormsg = "send failed";
    }
}


void connection::send_login(std::string user, std::string pass) {
    clearBuffer();
    klcp response;
    response.set("type", "login");
    response.set("username", user);
    response.set("password", pass);

    if (response.send(&connection_socket)) {
        error = false;
    } else {
        error = true;
        errormsg = "send failed";
    }
}

void connection::send_file(std::string filename, std::string path) {
    clearBuffer();

    std::stringstream ss;
    ss << path << "/" << filename;
    std::string file = ss.str();
    ss.clear();

    std::ifstream filetosend;
    filetosend.open(file.c_str(), std::ifstream::binary);

    if (filetosend) {

        std::ifstream getfilesize;
        getfilesize.open(file.c_str(), std::ios::ate);

        unsigned long filesize = (unsigned long) getfilesize.tellg();
        unsigned long blockcount = (unsigned long) ceil((float) filesize / FILEBUF);
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

        if (response.send(&connection_socket)) {
            char FileBuffer[FILEBUF];

            for (unsigned long block = 0; block < blockcount; block++) {

                unsigned long blocksize = FILEBUF;
                if (block == (blockcount - 1)) {
                    blocksize = lastBlockSize;
                };

                float progress = ((float) block / (float) blockcount);

                int barWidth = 60;

                std::cout << BOLDGREEN << "Uploading... [";
                int pos = (int) round(barWidth * progress);
                if (block == blockcount - 1) {
                    for (int i = 0; i < barWidth; ++i) {
                        std::cout << "#";
                    }
                    std::cout << "] 100%\r" << RESET;
                    std::cout.flush();
                } else {
                    for (int i = 0; i < barWidth; ++i) {
                        if (i <= pos) std::cout << "#";
                        else std::cout << "-";
                    }
                    std::cout << "] " << round(progress * 100.0) << "%\r" << RESET;
                    std::cout.flush();
                }


                filetosend.read(FileBuffer, blocksize);
                writen(connection_socket, FileBuffer, blocksize);
            }

            filetosend.close();


            std::cout << std::endl << std::endl;

            printInfo("File Upload Finished");
            error = false;
        } else {
            error = true;
            errormsg = "file send failed";
        }
    } else {
        printError("File not Found!");
        send_message("File not found");
        skip = true;
    }
}

void connection::getFile(klcp request, std::string filepath) {

    std::stringstream ss;
    ss << filepath << "/" << request.get("filename");
    std::string file = ss.str();
    ss.clear();

    std::ofstream filetosave;

    std::cout << std::endl;

    filetosave.open(file.c_str(), std::ofstream::binary);

    unsigned long _blocksize = request.getLong("blocksize");
    unsigned long blockcount = request.getLong("blockcount");
    unsigned long lastBlockSize = request.getLong("lastblocksize");

    char FileBuffer[FILEBUF];

    for (unsigned long block = 0; block < blockcount; block++) {

        unsigned long blocksize = _blocksize;
        if (block == (blockcount - 1)) {
            blocksize = lastBlockSize;
        };

        float progress = ((float) block / (float) blockcount);

        int barWidth = 60;

        std::cout << BOLDGREEN << "Downloading... [";
        int pos = (int) round(barWidth * progress);
        if (block == blockcount - 1) {
            for (int i = 0; i < barWidth; ++i) {
                std::cout << "#";
            }
            std::cout << "] 100%\r" << RESET;
            std::cout.flush();
        } else {
            for (int i = 0; i < barWidth; ++i) {
                if (i <= pos) std::cout << "#";
                else std::cout << "-";
            }
            std::cout << "] " << round(progress * 100.0) << "%\r" << RESET;
            std::cout.flush();
        }


        if (readn(connection_socket, FileBuffer, blocksize) == -1) {
            error = true;
            errormsg = "failed to read file";
            break;
        } else {
            filetosave.write(FileBuffer, blocksize);
        }
    }

    std::cout << std::endl;
    printInfo("File Download Finished");
    error = false;
}

void connection::close_connection() {
    close(connection_socket);
}