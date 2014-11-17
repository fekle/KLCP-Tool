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

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#include <termios.h>
#include <unistd.h>


std::string filesInDir(std::string path){
    std::string files;
    
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL){
        files = "Available Files:\n";
        int i = 0;
        while ((ent = readdir (dir)) != NULL){
            if(i>1){
                files += "\n";
                files += ent->d_name;
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

void printError(std::string msg) {
    std::cout << std::endl;
    std::cout << BOLDRED << "|==================== ERROR ===================|" << std::endl;
    std::cout << " " << msg << std::endl;
    std::cout << "|==============================================|" << RESET;
    std::cout << std::endl << std::endl;
}

void printInfo(std::string msg) {
    std::cout << std::endl;
    std::cout << BOLDYELLOW << "|==================== INFO ====================|" << std::endl;
    std::cout << " " << msg << std::endl;
    std::cout << "|==============================================|" << RESET;
    std::cout << std::endl << std::endl;
}

void printMsg(std::string msg) {
    std::cout << std::endl;
    std::cout << BOLDBLUE << "|==================== MESSAGE =================|" << std::endl;
    std::cout << " " << msg << std::endl;
    std::cout << "|==============================================|" << RESET;
    std::cout << std::endl << std::endl;
}

#endif
