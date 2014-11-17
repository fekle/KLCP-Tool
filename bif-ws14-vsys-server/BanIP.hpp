//
//  BanIP.h
//  bif-ws14-vsys-server
//
//  Created by Aleksandar Lepojic on 15.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#ifndef bif_ws14_vsys_server_BanIP_hpp
#define bif_ws14_vsys_server_BanIP_hpp

#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "common.hpp"

/**
* IP Class
* Define time variables and handling of IP
*/

class IP {

private:
    time_t ban_time;
    std::string ip_address;

public:
    IP(std::string IP, time_t duration);

    time_t getTimestamp();

    std::string getIP();
};

IP::IP(std::string _ip, time_t _duration) {

    time_t time_now = time(0);

    time_now += _duration;

    ip_address = _ip;
    ban_time = time_now;
}

std::string IP::getIP() {
    return ip_address;
}

time_t IP::getTimestamp() {
    return ban_time;
}

/*
 * banned_ip Class
 * Define two bool methods for banning.
 */

class banned_ip {
private:
    std::vector<IP> ip_s;
public:
    banned_ip();

    bool the_ban(std::string);

    bool is_banned(std::string, time_t);
};

banned_ip::banned_ip() {

}

/**
* Methods which takes care of the banning process.
* Banned IP is being pushed into a vector data structure.
* Banning time is set to one minute.
* A proper comparison is set to unban the IP after the banning time has expired.
*/
bool banned_ip::the_ban(std::string _ip) {

    time_t time_right_now;
    time(&time_right_now);

    time_t time_difference;

    int bancount = (int) ip_s.size();


    if (bancount > 0) {
        for (int i = 0; i <= bancount; i++) {
            printInfo("FORLOOOP 2");

            if (ip_s[i].getIP() == _ip) {

                time_difference = ip_s[i].getTimestamp() - time_right_now;

                if (time_difference > 0) {
                    std::stringstream ss;
                    ss << "IP (" << ip_s[i].getIP() << ") is banned!";
                    printError(ss.str());
                    ss.clear();

                    return true;
                } else {
                    int x = i - 1;
                    ip_s.erase(ip_s.begin() + x);
                    return false;
                }
            }
        }
    }

    return false;
}

bool banned_ip::is_banned(std::string _ip, time_t _duration) {
    std::stringstream ss;
    ss << "Banning IP (" << _ip << ") for " << std::to_string(_duration) << " minutes.";
    printError(ss.str());
    ss.clear();

    if (_duration < 0) {
        return false;
    }
    _duration = _duration * 60;
    IP tmp_ip(_ip, _duration);
    ip_s.push_back(tmp_ip);

    printError("IP Banned");

    return true;
}

#endif