//
//  common.hpp
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#ifndef bif_ws14_vsys_server_common_hpp
#define bif_ws14_vsys_server_common_hpp

#include <dirent.h>

std::string filesInDir(std::string path){
    std::string files;
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL){
        files = "Available Files:\n";
        int i = 0;
        while ((ent = readdir (dir)) != NULL){
            if(i>1){
                files += ent->d_name;
                files += " ";
            }
            i++;
        }
        closedir (dir);
    }else{
        files = "Directory not found";
    }
    
    return files;
};

#endif
