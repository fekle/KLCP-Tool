/**
* common.hpp
* commonly used functions and macros
*
* Created by Aleksandar Lepojic & Felix Klein, 2014
*/

#ifndef bif_ws14_vsys_server_common_hpp
#define bif_ws14_vsys_server_common_hpp

#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <sys/stat.h>

/**
* Macros for Colored output.
*/
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

/**
* Human readable FileSize (from http://programanddesign.com/cpp/human-readable-file-size-in-c/)
*/
std::string readable_fs(double size/*in bytes*/) {
    int i = 0;
    const char *units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1000) {
        size /= 1000;
        i++;
    }
    char buf[100];
    sprintf(buf, "%.*f %s", i, size, units[i]);
    return std::string(buf);
}

std::string readable_fs_i(double size/*in bytes*/) {
    int i = 0;
    const char *units[] = {"B", "kiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    char buf[100];
    sprintf(buf, "%.*f %s", i, size, units[i]);
    return std::string(buf);
}

/**
* List the files in a directory
*/
std::string filesInDir(std::string path) {
    std::stringstream files;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path.c_str())) != NULL) {
        files << "Available Files:" << std::endl;
        int i = 0;
        while ((ent = readdir(dir)) != NULL) {
            if (i > 1) {
                std::string name(ent->d_name);
                if (name[0] != '.') {
                    std::stringstream fullpath;
                    fullpath << path << "/" << name;
                    struct stat info;
                    lstat(fullpath.str().c_str(), &info);
                    std::string filesize = readable_fs(info.st_size);
                    std::string filesize_i = readable_fs_i(info.st_size);
                    files << std::endl << " " << BOLDYELLOW << name << BOLDBLUE << "\t - " << filesize << " (" << filesize_i << ")";
                }
            }
            i++;
        }
        closedir(dir);
    } else {
        files << "Directory not found";
    }

    return files.str();
};


/**
* readn function (from http://www.informit.com/articles/article.aspx?p=169505&seqNum=9)
* ensures ALL bytes are read
*/
ssize_t readn(int fd, char *vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
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

/**
* writen function (from http://www.informit.com/articles/article.aspx?p=169505&seqNum=9)
* ensures ALL bytes are written
*/
ssize_t writen(int fd, const char *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
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

/**
* Function for printing Errors
*/
void printError(std::string msg) {
    std::cerr << std::endl;
    std::cerr << BOLDRED << "|==================== ERROR ===================|" << std::endl;
    std::cerr << " " << msg << std::endl;
    std::cerr << "|==============================================|" << RESET;
    std::cerr << std::endl << std::endl;
}

/**
* Function for printing Infos
*/
void printInfo(std::string msg) {
    std::cout << std::endl;
    std::cout << BOLDYELLOW << "|==================== INFO ====================|" << std::endl;
    std::cout << " " << msg << std::endl;
    std::cout << "|==============================================|" << RESET;
    std::cout << std::endl << std::endl;
}

/**
* Function for printing Messages
*/
void printMsg(std::string msg) {
    std::cout << std::endl;
    std::cout << BOLDBLUE << "|==================== MESSAGE =================|" << std::endl;
    std::cout << " " << msg << std::endl;
    std::cout << "|==============================================|" << RESET;
    std::cout << std::endl << std::endl;
}

#endif
