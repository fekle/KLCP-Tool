//
//  connection.hpp
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#include "ldap_connect.hpp"

#ifndef bif_ws14_vsys_server_connection_hpp
#define bif_ws14_vsys_server_connection_hpp

void *serverThread(int, std::string);

void server(int port, std::string filepath) {

    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;

    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(listen_socket, (struct sockaddr *) &address, sizeof(address)) != 0) {
        std::stringstream error;
        error << "BIND ERROR - Could not bind to Port " << port << ": " << strerror(errno);
        printError(error.str());
    } else {
        listen(listen_socket, 5);
        addrlen = sizeof(struct sockaddr_in);
    }

    do {

        int sock = accept(listen_socket, (struct sockaddr *) &cliaddress, &addrlen);
        if (sock > 0) {
            std::stringstream info;
            info << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port);
            printInfo(info.str());

            std::thread x(serverThread, sock, filepath);
            x.detach();
        } else {
            printError("Connection Failed!");
        }


    } while (true);

    close(listen_socket);
}

void *serverThread(int sock, std::string filepath) {
    int id = rand() % 1000 + 1;
    std::string command;
    connection conn(sock);
    conn.send_message("Welcome to MyServer! Please Log In!");

    do {

        klcp login = conn.recieve();

        if (conn.error) {
            break;
        }

        if (login.get("type") == "login") {

            ldap_auth ldap;

            std::string user = login.get("username");
            std::string pass = login.get("password");

            bool valid = ldap.establish_ldap_auth(user, pass);

            if (valid) {

                conn.send_message("Welcome!");

                if (conn.error) {
                    break;
                }

                do {
                    command = "";
                    std::stringstream hi;
                    hi << "Waiting for Client in thread #" << id;
                    printInfo(hi.str());

                    klcp msg = conn.recieve();

                    if (conn.error) {
                        break;
                    }

                    std::string requestType = msg.get("type");

                    if (requestType == "message") {
                        printMsg(msg.get("msg"));
                    } else if (requestType == "command") {
                        command = msg.get("command");
                        std::stringstream info;
                        info << "Client sent command: " << command << " " << msg.get("value");
                        printInfo(info.str());
                        if (command == "LIST") {
                            conn.send_message(filesInDir(filepath));
                        } else if (command == "GET") {
                            conn.send_file(msg.get("value"), filepath);
                        } else {
                            conn.send_message("Invalid command!");
                        }
                    } else if (requestType == "file") {
                        conn.getFile(msg, filepath);
                        conn.send_message("got it");
                    }

                    conn.clearBuffer();

                } while (command != "QUIT" && command != "QUITALL" && !conn.error);
            } else {
                conn.send_message("Invalid Credentials");
            }
        } else {
            conn.send_message("Please Log in");
        }

    } while (command != "QUIT" && command != "QUITALL" && !conn.error);

    conn.close_connection();

    return 0;
}

#endif
