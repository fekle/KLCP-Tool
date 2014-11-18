/**
* server.hpp
* Implements the Server
*/

#include "ldap_connect.hpp"
#include "BanIP.hpp"

#ifndef bif_ws14_vsys_server_server_hpp
#define bif_ws14_vsys_server_server_hpp

/**
* Define ServerThread prototype
*/
void *serverThread(int, std::string, std::string, banned_ip *banservice);

/**
* Function for server, takes the port and filepath as arguments
*/
void server(int port, std::string filepath) {

    /**
    * Typical
    */
    socklen_t addrlen;
    struct sockaddr_in address, cliaddress;

    /**
    * Create listening socket
    */
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    /**
    * Error handling
    * If binding is successful, listen
    */
    if (bind(listen_socket, (struct sockaddr *) &address, sizeof(address)) != 0) {
        std::stringstream error;
        error << "BIND ERROR - Could not bind to Port " << port;
        printError(error.str());
    } else {
        listen(listen_socket, 5);
        addrlen = sizeof(struct sockaddr_in);
    }

    banned_ip banservice;

    do {
        /**
        * Accept requests
        */
        int sock = accept(listen_socket, (struct sockaddr *) &cliaddress, &addrlen);

        std::stringstream ss;
        ss << inet_ntoa(cliaddress.sin_addr);
        std::string clientIP = ss.str();
        ss.clear();

        /**
        * Error Handling
        */
        if (sock > 0) {
            std::stringstream info;
            info << "Client connected from " << inet_ntoa(cliaddress.sin_addr) << ":" << ntohs(cliaddress.sin_port);
            printInfo(info.str());

            /**
            * Pass socket and filepath to Thread, and detach
            */
            std::thread x(serverThread, sock, filepath, clientIP, &banservice);
            x.detach();
        } else {
            printError("Connection Failed!");
        }

        /**
        * Endless loop, thats bad, we know that.
        */
    } while (true);

    /**
    * Close socket
    */
    close(listen_socket);
}

/**
* Funvtion for server Thread
* Takes a socket and filepath as arguments
*/
void *serverThread(int sock, std::string filepath, std::string clientIP, banned_ip *banservice) {
    /**
    * Calculate random Number for Distinction of threads
    */
    int id = rand() % 1000 + 1;
    std::string command;

    /**
    * Create new connection object
    */
    connection conn(sock);

    bool isbanned = banservice->the_ban(clientIP);

    if (isbanned) {
        conn.send_command("ban");
        conn.close_connection();
        return 0;
    }

    /**
    * Tell the client to login
    */
    conn.send_message("Welcome to MyServer! Please Log In!");

    /**
    * Crate new ldap_auth object
    */
    ldap_auth ldap;


    do {


        /**
        * Recieve login object
        */
        klcp login = conn.recieve();

        if (conn.error) {
            break;
        }

        /**
        * If request isn't login request, tell the Client to login again and wait.
        * If it is a login request, continue
        */
        if (login.get("type") == "login") {

            /**
            * Get username and password
            */
            std::string user = login.get("username");
            std::string pass = login.get("password");


            /**
            * Check if credentials are valid (true = valid, false = invalid)
            */
            bool valid = ldap.establish_ldap_auth(user, pass, clientIP, banservice);

            /**
            * For Debugging, and using from outside FH
            */


            /**
            * If invalid, tell the user and wait for new LOGIN request
            */
            if (valid) {

                /**
                * I'm a friendly Server
                */
                conn.send_message("Welcome! How are you?");

                if (conn.error) {
                    break;
                }

                do {
                    command = "";
                    std::stringstream hi;
                    hi << "Waiting for Client in thread #" << id;
                    printInfo(hi.str());

                    /**
                    * get Request from server
                    */
                    klcp request = conn.recieve();

                    if (conn.error) {
                        break;
                    }

                    std::string requestType = request.get("type");

                    /**
                    * Switch through request types
                    */
                    if (requestType == "message") {
                        /**
                        * If message, print it
                        */
                        printMsg(request.get("msg"));
                    } else if (requestType == "command") {
                        /**
                        * If command, handleit
                        */
                        command = request.get("command");
                        std::stringstream info;
                        info << "Client sent command: " << command << " " << request.get("value");
                        printInfo(info.str());

                        /**
                        * Switch through commands
                        */
                        if (command == "LIST") {
                            /**
                            * Send List of Files
                            */
                            conn.send_message(filesInDir(filepath));
                        } else if (command == "GET") {
                            /**
                            * Send file
                            */
                            conn.send_file(request.get("value"), filepath);
                        } else {
                            /**
                            * If command type is invalid, tell the Client
                            */
                            conn.send_message("Invalid command!");
                        }
                    } else if (requestType == "file") {
                        /**
                        * If requestType is "file", send the file
                        */
                        conn.getFile(request, filepath);
                        conn.send_message("got it");
                    }

                    /**
                    * Do while command isn't Quit and there is no Error
                    */
                } while (command != "QUIT" && !conn.error);
            } else {
                /**
                * Tell the Client that the LOGIN credentials are invalid
                */
                isbanned = banservice->the_ban(clientIP);

                if (isbanned) {
                    conn.send_command("ban");
                    conn.close_connection();
                    return 0;
                } else {
                    conn.send_message("Invalid Credentials. Sorry.");
                }
            }
        } else {
            /**
            * Tell the Client to log in
            */
            conn.send_message("Please Log in");
        }

        /**
        * Do while command isn't Quit and there is no Error
        */
    } while (command != "QUIT" && !conn.error);

    /**
    * close Socket
    */
    conn.close_connection();

    return 0;
}

#endif
