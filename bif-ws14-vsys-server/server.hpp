//
//  connection.hpp
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#ifndef bif_ws14_vsys_server_connection_hpp
#define bif_ws14_vsys_server_connection_hpp

void server(int port, std::string filepath){
    connection conn(port);
    
    while(1){
        std::cout << "Waiting for connections..." << std::endl;
        conn.new_server_connection();
        conn.send_message("HI");
        
        do{
            std::cout << std::endl << "Wating for message... " << std::endl;
            conn.recieve_message();
            std::string requestType = conn.request.getString("type");
            
            if(requestType == "message"){
                std::cout << conn.request.getString("msg") << std::endl;
            }else if(requestType == "command"){
                std::string command = conn.request.getString("msg");
                if(command == "LIST"){
                    conn.send_message(filesInDir(filepath));
                }else if (command == "GET"){
                    conn.send_file(conn.request.getString("value"), filepath);
                }else if(command == "QUIT"){
                    conn.send_message("BYE!");
                }else{
                    conn.send_message("Invalid command!");
                }
            }else if(requestType == "file"){
                conn.getFile(conn.request, filepath);
            }
            
        }while(conn.request.getString("msg") != "QUIT");
        
        conn.close_connection();
        
        if(strncmp(conn.buffer, "quitall", 7)  == 0){
            std::cout << "Client told me to quit, I obey." << std::endl;
            break;
        }
    }
    
    conn.quit_server();
}

#endif
