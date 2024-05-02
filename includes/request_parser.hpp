#pragma once
#include <iostream>
#include <sstream>
#include <fstream> 
class request_data
{
    private:
        std::string request_text;
        std::string method;
        std::string target;
        std::string http_version;
        std::string host;
        int port;
        std::string user_agent;
        std::string accept;
        std::string accept_language;
        std::string accept_encoding;
        std::string connection;
        int content_length;
        std::string content_type;
        std::string body;
        int status_line;
        std::string cgi_bin;
        std::string boundary;
    public:
        //construct
        //request_data();
        request_data(std::string type, std::string host_directory, std::string cgi_directory);
        int parse_method();
        int parse_target(std::string host_directory, std::string cgi_directory);
        int parse_version();
        int parse_headers();

        std::string get_method();
        std::string get_target();
        std::string get_http_version();
        std::string get_host();
        int get_port();
        std::string get_user_agent();
        std::string get_accept();
        std::string get_accept_language();
        std::string get_accept_encoding();
        std::string get_connection();
        int get_status_line();
        int get_content_length();
        std::string get_content_type();
        std::string get_body();
        std::string get_cgi_bin();
        //copy construct
        //request_data(const Animal& original);

        // //copy operator assign
            //request_data& operator=(const request_data& original);

        //destructor
        //virtual ~request_data();
};

int	ft_atoi(const char *str);