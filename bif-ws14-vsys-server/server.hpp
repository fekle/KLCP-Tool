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
        
        do{
            std::cout << std::endl << "Wating for message... " << std::endl;
            conn.recieve_message();
            std::string requestType = conn.request.getString("type");
            
            if(requestType == "LIST"){
                conn.send_message(filesInDir(filepath));
            }else if(requestType == "GET"){
                conn.send_file(conn.request.getString("path"), filepath);
            }else if(requestType == "QUIT"){
                conn.send_message("BYE!");
            }else{
                conn.send_message("Invalid Command");
            }
            
            
        }while(conn.request.getString("type") != "QUIT");
        
        conn.close_connection();
        
        if(strncmp(conn.buffer, "quitall", 7)  == 0){
            std::cout << "Client told me to quit, I obey." << std::endl;
            break;
        }
    }
    
    conn.quit_server();
}

#endif
