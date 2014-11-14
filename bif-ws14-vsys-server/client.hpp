//
//  client.hpp
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#ifndef bif_ws14_vsys_server_client_hpp
#define bif_ws14_vsys_server_client_hpp
#define BUF 1024

void client(std::string adress, int port, std::string filepath){
    connection conn;
    conn.new_client_connection(port, adress);

    
    do{
        
        std::string cmd = "";
        std::string cmd2 = "";
        
        conn.recieve_message();
        
        std::string requestType = conn.request.getString("type");
        
        if(requestType == "message"){
            std::cout << std::endl << "Message by Server:" << std::endl << "===" << std::endl << "" << conn.request.getString("msg") << std::endl << "===" << std::endl;
        }else if(requestType == "command"){
            std::string command = conn.request.getString("msg");
            if(command == "LIST"){
               // conn.send_message(filesInDir(filepath));
            }else if (command == "GET"){
                //conn.send_file(conn.request.getString("msg"), filepath);
            }else if(command == "QUIT"){
                //conn.send_message("BYE!");
            }else{
                //conn.send_message("Invalid command!");
            }
        }else if(requestType == "file"){
            conn.getFile(conn.request, filepath);
        }
        
        std::cout << "Enter Command: [cmd] [val]" << std::endl << ">\t";
        
        std::cin >> cmd >> cmd2;
        
        if(cmd == "LIST"){
            conn.send_command("LIST", cmd2);
        }else if(cmd == "GET"){
            conn.send_command("GET", cmd2);
        }else if(cmd == "PUT"){
             conn.send_file(cmd2, filepath);
        }else if(cmd == "QUIT"){
            break;
        }
        
        
    }while(true);
}

#endif
