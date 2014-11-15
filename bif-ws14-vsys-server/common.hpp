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

ssize_t                         /* Read "n" bytes from a descriptor. */
readn(int fd, char *vptr, size_t n)
{
    size_t  nleft;
    ssize_t nread;
    char   *ptr;
    
    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;      /* and call read() again */
            else
                return (-1);
        } else if (nread == 0)
            break;              /* EOF */
        
        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

ssize_t                         /* Write "n" bytes to a descriptor. */
writen(int fd, const char *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;
    
    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;   /* and call write() again */
            else
                return (-1);    /* error */
        }
        
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

#endif
