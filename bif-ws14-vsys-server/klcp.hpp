//
//  klcp.h
//  bif-ws14-vsys-server
//
//  Created by Felix Klein on 13.11.14.
//  Copyright (c) 2014 Aleksandar Lepojic, Felix Klein. All rights reserved.
//

#ifndef __bif_ws14_vsys_server__klcp__
#define __bif_ws14_vsys_server__klcp__

#include <stdio.h>
#include <string>
#include <map>
#include <strstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::property_tree;

class klcp{
private:
    ptree msg;
public:
    klcp();
    void setInt(std::string, int);
    void setString(std::string, std::string);
    void setDouble(std::string, double);
    
    int getInt(std::string);
    std::string getString(std::string);
    double getDouble(std::string);
    
    void msgParse(std::string);
    std::string msgSerialize();
};

klcp::klcp(){
 //   std::cout << "BANANE OIDA";
}

void klcp::setInt(std::string _key, int _val){
    msg.put(_key, _val);
}

void klcp::setString(std::string _key, std::string _val){
    msg.put(_key, _val);
}

void klcp::setDouble(std::string _key, double _val){
    msg.put(_key, _val);
}

int klcp::getInt(std::string _key){
    return msg.get<int>(_key);
}

std::string klcp::getString(std::string _key){
    return msg.get<std::string>(_key);
}

double klcp::getDouble(std::string _key){
    return msg.get<double>(_key);
}

void klcp::msgParse(std::string _json){
    std::strstream is;
    is << _json;
    try{
        read_json(is, msg);
    }catch(std::exception const& e){
        std::cout << _json;
        std::cout << "KLCP PARSE ERROR: " << e.what() << std::endl;
    }
}

std::string klcp::msgSerialize(){
    msg.put("protocol", "klcp");
    msg.put("version", "0.0.1");
  //  msg.put("length", msg.get<std::string>("msg").size());
    std::strstream _json;
    try{
        write_json(_json, msg);
    }catch(std::exception const& e){
        std::cout << "KLCP SERIALIZE ERROR: " << e.what() << std::endl;
    }
    return _json.str();
}

#endif /* defined(__bif_ws14_vsys_server__klcp__) */
