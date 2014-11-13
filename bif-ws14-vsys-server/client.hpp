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
    connection conn;
    conn.new_client_connection(port, adress);
}

#endif
