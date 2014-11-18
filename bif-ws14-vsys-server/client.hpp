/**
* client.hpp
* Implements the client function
* Copyright (c) 2014 Felix Klein, Aleksandar Lepojic. All rights reserved.
*/

#include <sys/termios.h>

#ifndef bif_ws14_vsys_server_client_hpp
#define bif_ws14_vsys_server_client_hpp

/**
* Function for client
*/
void client(std::string adress, int port, std::string filepath) {

    /**
    * create socket, the usual stuff
    */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    /**
    * create a new_client_connection with the socket, port and ip
    */
    connection conn(sock);
    conn.new_client_connection(port, adress);

    std::string cmd = "";
    std::string cmd2 = "";
    std::string cmd3 = "";

    /**
    * Error handling
    */
    if (!conn.error) {
        do {
            cmd = "";
            cmd2 = "";
            cmd3 = "";

            /**
            * If skip is true, don't wait for server response, if false, get message from server
            */
            if (!conn.skip) {
                /**
                * Create new klcp object
                */
                klcp msg = conn.recieve();

                if (conn.error) {
                    break;
                }

                /**
                * Get request Type. if message then print message, if file then start getFile.
                */
                std::string requestType = msg.get("type");

                if (requestType == "message") {
                    printMsg(msg.get("msg"));
                } else if (requestType == "file") {
                    conn.getFile(msg, filepath);
                } else if (requestType == "command") {
                    if (msg.get("command") == "ban") {
                        printError("You are Banned! Sorry...");
                        break;
                    }
                }
            }

            /**
            * Ask user for command
            */
            std::cout << BOLDWHITE << "Enter Command:" << std::endl << RESET << BOLDCYAN << "\t> " << RESET;

            /**
            * Get command
            */
            std::cin >> cmd;

            /**
            * Switch through command types
            */
            if (cmd == "LOGIN") {
                /**
                * LOGIN command
                * First ask for the password
                */
                std::cin >> cmd2;
                std::cout << "Enter Password:" << std::endl << "\t>";

                /**
                * Set the terminal to no-echo, so the password text doesn't get displayed, then read password, then restore terminal echoing
                */
                struct termios oldt, newt;
                tcgetattr(STDIN_FILENO, &oldt);
                newt = oldt;
                newt.c_lflag &= ~(ICANON | ECHO);
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
                std::cin >> cmd3;
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

                /**
                * Send login request
                */
                conn.send_login(cmd2, cmd3);
            } else if (cmd == "LIST") {
                /**
                * Send LIST command
                */
                conn.send_command("LIST", "/");
            } else if (cmd == "GET") {
                /**
                * Read second argument (filename) and send command
                */
                std::cin >> cmd2;
                conn.send_command("GET", cmd2);
            } else if (cmd == "PUT") {
                /**
                * Same as above
                */
                std::cin >> cmd2;
                conn.send_file(cmd2, filepath);
            } else if (cmd == "QUIT") {
                /**
                * Quit
                */
                conn.send_command("QUIT");
                printInfo("Bye!");
            } else {
                /**
                * Set skip to true, and tell the user that the command is unknown
                */
                conn.skip = true;
                printError("Invald Command!");
            }

            /**
            * Loop, Quit if user enters QUIT, or ther is a unresolved error
            */
        } while (cmd != "QUIT" && !conn.error);

        /**
        * Close connection
        */
        conn.close_connection();

    } else {
        printError("Connection Failed!");
    }
}

#endif
