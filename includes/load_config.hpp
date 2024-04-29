#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdlib> // For atoi

class RouteConfig {
public:
    std::string path;
    std::string root_directory;
    std::string default_file;
    bool list_directory;
    std::vector<std::string> accepted_methods;
    std::string redirect;
    bool cgi_enable;
    std::string cgi_path;
    std::string cgi_extensions;
    bool upload_enable;
    std::string upload_path;

    RouteConfig() : list_directory(false), cgi_enable(false), upload_enable(false) {}
};

class ServerConfig {
public:
    std::string host;
    int port;
    std::vector<std::string> server_names;
    std::map<int, std::string> default_error_pages;
    std::string limit_client_body_size;
    std::vector<RouteConfig> routes;

    ServerConfig() : port(0) {}
};

class CGIConfig {
public:
    std::string cgi_bin_path;
    std::string php_cgi;
    std::string python_cgi;
    std::string cgi_executable_extensions;
};

class WebServerConfig {
public:
    std::vector<ServerConfig> servers;
    CGIConfig cgi_config;
    std::string chunk_handling;

    void parseConfig(const std::string& filename);
};
void execute_cgi(const std::string& script_path, const std::string& post_data);

void trim(std::string& s);