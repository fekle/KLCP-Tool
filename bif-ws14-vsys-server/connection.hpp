/**
* connection.hpp
* Handles the connection funcitons like sending and recieving different types of informations - works together with klcp.hpp
*
* Created by Aleksandar Lepojic & Felix Klein, 2014
*/

#ifndef bif_ws14_vsys_server_connection_hpp
#define bif_ws14_vsys_server_connection_hpp

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

/**
* Define buffers
* The normal message buffer is 1000 bytes,
* the file buffer is 100 kilobytes (used for sending/recieving files)
*/
#define BUF 1000
#define FILEBUF 100000

/**
* Main class for connections
*/
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

/**
* Function for clearing the buffer
*/
void connection::clearBuffer() {
    bzero(buffer, BUF);
}

/**
* Empty instantiation, used by client
*/
connection::connection() {
    clearBuffer();
}

/**
* Instantiation with socket, used by server
*/
connection::connection(int socket) {
    clearBuffer();
    connection_socket = socket;
}

/**
*  Function for establishing a new client connection, used by client (duh!)
*/
void connection::new_client_connection(int port, std::string _adress) {
    /**
    * Socket variables
    */
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_aton(_adress.c_str(), &address.sin_addr);

    /**
    * Try to connect, if it fails print a error message and set the error variable to TRUE
    */
    if (connect(connection_socket, (struct sockaddr *) &address, sizeof(address)) == 0) {
        error = false;
        printInfo("Connected!");
    } else {
        error = true;
        printError("Connection Failed!");
    }
}

/**
* Funciton for recieving from the server/client.
* Returns a klcp object.
*/
klcp connection::recieve() {
    klcp request;

    /**
    * error checking - klcp returns TRUE when reading is succesful, FALSE if not
    */
    if (request.recieve(&connection_socket)) {

        /**
        * check if Type is a message, if so, read the number of bytes defined by "msglength" in the header from the socket
        */
        if (request.get("type") == "message") {
            unsigned long msglength = (unsigned long) atol(request.get("length").c_str());
            char msgBuffer[msglength];

            readn(connection_socket, msgBuffer, (size_t) msglength);

            std::string msg(msgBuffer);

            /**
            * Resize message to the correct size, kinda "hacky", but works very well
            */
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

/**
* Function for sending a Message
*/
void connection::send_message(std::string message) {
    /**
    * Determine Message size, klcp object, set headers etc
    */
    unsigned long msglength = message.size();
    char msgBuffer[msglength];
    std::copy(message.begin(), message.end(), msgBuffer);
    klcp response;
    response.set("type", "message");
    response.set("length", std::to_string(msglength));

    /**
    * If sending the header is successful, write the message
    */
    if (response.send(&connection_socket)) {
        writen(connection_socket, msgBuffer, msglength);
    } else {
        error = true;
        errormsg = "send failed";
    }
}

/**
* Send command - very similar to send_message, but without sending the message (duh!) and a different "type" field in the header - "command"
*/
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

/**
* The same as above, but with just one variable. E.g for sending the LIST command
*/
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

/**
* Similar to the send_command functions, but with type "login" and passing the data entered by the user
*/
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

/**
* Our biggest function :)
* Function for sending a file.
* Takes the filename and the root path (supplied by the user at program start) as arguments
*/
void connection::send_file(std::string filename, std::string path) {
    /**
    * Construct a valid filepath out of the "path" and "filename" variables.
    * It is possible to end up as a string with two "/" somewhere, but POSIX doesn't care :)
    */
    std::stringstream ss;
    ss << path << "/" << filename;
    std::string file = ss.str();
    ss.clear();

    /**
    * Open the file in a ifstram, in binary mode
    */
    std::ifstream filetosend;
    filetosend.open(file.c_str(), std::ifstream::binary);

    /**
    * If the file exits continue
    */
    if (filetosend) {

        /**
        * Determine the file's size
        */
        struct stat info;
        lstat(file.c_str(), &info);
        unsigned long filesize = (unsigned long) info.st_size;

        /**
        * Determine the blockcount and the size of the last block (for chunked sending of the file)
        */
        unsigned long blockcount = (unsigned long) ceil((float) filesize / FILEBUF);
        unsigned long lastBlockSize = filesize % FILEBUF;

        /**
        * Create a new klcp object, and set the headers.
        *   type, filename, filesize, blocksize, blockcount and lastblocksize
        *   - Theoretically the client could calculate most of tose values himself, but for safety and convenience we send the values to him. Don't need to do the work twice.
        */
        klcp response;
        response.set("type", "file");
        response.set("filename", filename);
        response.setLong("filesize", filesize);
        response.setLong("blocksize", FILEBUF);
        response.setLong("blockcount", blockcount);
        response.setLong("lastblocksize", lastBlockSize);

        /**
        * If header is sent successfully
        */
        if (response.send(&connection_socket)) {
            /**
            * create filebuffer
            */
            char FileBuffer[FILEBUF];

            std::stringstream info2;
            info2 << "Sending file \"" << filename << "\" (" << readable_fs((double) filesize) << " / " << readable_fs_i((double) filesize) << ")";
            printInfo(info2.str());

            /**
            * loop through the blockcount, sending each block to the client
            */
            for (unsigned long block = 0; block < blockcount; block++) {
                unsigned long blocksize = FILEBUF;

                /**
                * if the block is the last block, then set the blocksize to lastblocksize, because the last block is smaller than the rest
                */
                if (block == (blockcount - 1)) {
                    blocksize = lastBlockSize;
                };

                /**
                * calculate the progress
                */
                float progress = ((float) block / (float) blockcount);

                /**
                * Display a progression bar.
                * This implementation use carriage return and flushing to replace the last line with a new one, to update the progress bar
                */
                std::cout << BOLDGREEN << "Uploading... [";
                int pos = (int) round(60 * progress);
                /**
                * If we are in the last block, then always set progress to 100, because sometimes, when we have small files, the bar would stop at values like 98%,
                *   even though the file send has finished
                */
                if (block == blockcount - 1) {
                    for (int i = 0; i < 60; ++i) {
                        std::cout << "#";
                    }
                    std::cout << "] 100%\r" << RESET;
                    std::cout.flush();
                } else {
                    for (int i = 0; i < 60; ++i) {
                        if (i <= pos) std::cout << "#";
                        else std::cout << "-";
                    }
                    std::cout << "] " << round(progress * 100.0) << "%\r" << RESET;
                    std::cout.flush();
                }

                /**
                * Read block from file and write it to socket, with error handling
                */
                filetosend.read(FileBuffer, blocksize);
                ssize_t x = writen(connection_socket, FileBuffer, blocksize);
                if (x <= 0) {
                    error = true;
                    errormsg = "failed to write to socket";
                    break;
                }
            }

            /**
            * close the file
            */
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
        /**
        * File not found is not a error in our case, we just tell the client to skip, meaning he doesn't wait for messages by the server, and asks for a new command
        *   immediately
        */
        skip = true;
    }
}

/**
* The opposite function of sendFile()
*   We aren't going to explain everything here, because many parts are similar to the sendFile() function
*/
void connection::getFile(klcp request, std::string filepath) {

    std::stringstream ss;
    ss << filepath << "/" << request.get("filename");
    std::string file = ss.str();
    ss.clear();

    /**
    * Open/Create target file as ofstream in binary mode
    */
    std::ofstream filetosave;
    filetosave.open(file.c_str(), std::ofstream::binary);

    /**
    * get blocksize, blockcount and lastBlockSize from the klcp send by the server
    */
    unsigned long _blocksize = request.getLong("blocksize");
    unsigned long blockcount = request.getLong("blockcount");
    unsigned long lastBlockSize = request.getLong("lastblocksize");
    std::string filename = request.get("filename");
    unsigned long filesize = request.getLong("filesize");

    std::stringstream info;
    info << "Recieving file \"" << filename << "\" (" << readable_fs((double) filesize) << " / " << readable_fs_i((double) filesize) << ")";
    printInfo(info.str());

    char FileBuffer[FILEBUF];

    for (unsigned long block = 0; block < blockcount; block++) {

        unsigned long blocksize = _blocksize;
        if (block == (blockcount - 1)) {
            blocksize = lastBlockSize;
        };

        float progress = ((float) block / (float) blockcount);

        std::cout << BOLDGREEN << "Downloading... [";
        int pos = (int) round(60 * progress);
        if (block == blockcount - 1) {
            for (int i = 0; i < 60; ++i) {
                std::cout << "#";
            }
            std::cout << "] 100%\r" << RESET;
            std::cout.flush();
        } else {
            for (int i = 0; i < 60; ++i) {
                if (i <= pos) std::cout << "#";
                else std::cout << "-";
            }
            std::cout << "] " << round(progress * 100.0) << "%\r" << RESET;
            std::cout.flush();
        }

        /**
        * read file from socket and write to file, do obvious error handling
        */
        ssize_t x = readn(connection_socket, FileBuffer, blocksize);
        if (x <= 0) {
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

/**
* Function for closing the socket
*/
void connection::close_connection() {
    close(connection_socket);
}

#endif