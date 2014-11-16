//
//  client.hpp
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#ifndef bif_ws14_vsys_server_client_hpp
#define bif_ws14_vsys_server_client_hpp

void client(std::string adress, int port, std::string filepath){

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    connection conn(sock);
    conn.new_client_connection(port, adress);

    std::string cmd = "";
    std::string cmd2 = "";
    std::string cmd3 = "";

    if (!conn.error) {
        printInfo("Connected!");

        do {
            cmd = "";
            cmd2 = "";
            cmd3 = "";

            if (!conn.skip) {
                klcp msg = conn.recieve();

                if (conn.error) {
                    break;
                }

                std::string requestType = msg.get("type");

                if (requestType == "message") {
                    printMsg(msg.get("msg"));
                }
                if (requestType == "file") {
                    conn.getFile(msg, filepath);
                }
            }

            std::cout << BOLDWHITE << "Enter Command:" << std::endl << RESET << BOLDCYAN << "\t> " << RESET;

            std::cin >> cmd;

            if (cmd == "LOGIN") {
                std::cin >> cmd2;
                std::cout << "Enter Password:" << std::endl << "\t>";

                struct termios oldt, newt;
                tcgetattr(STDIN_FILENO, &oldt);
                newt = oldt;
                newt.c_lflag &= ~(ICANON | ECHO);
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
                std::cin >> cmd3;
                tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

                conn.send_login(cmd2, cmd3);
            } else if (cmd == "LIST") {
                std::cin >> cmd2;
                conn.send_command("LIST", cmd2);
            } else if (cmd == "GET") {
                std::cin >> cmd2;
                conn.send_command("GET", cmd2);
            } else if (cmd == "PUT") {
                std::cin >> cmd2;
                conn.send_file(cmd2, filepath);
            } else if (cmd == "QUIT") {
                conn.send_command("QUIT");
                printInfo("Bye!");
            } else if (cmd == "QUITALL") {
                conn.send_command("QUITALL");
                printInfo("Bye!");
            }else{
                conn.skip = true;
                printError("Invald Command!");
            }

            printInfo("sent");

            conn.clearBuffer();

        } while (cmd != "QUIT" && cmd != "QUITALL" && !conn.error);

        conn.close_connection();

    } else {
        printError("Connection Failed!");
    }
}

#endif
