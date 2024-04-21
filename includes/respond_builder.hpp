#include <iostream>
#include <sstream>
#include <fstream> 
#include "../includes/request_parser.hpp"

class respond_get
{
    private:
        request_data *request_info;
        std::string respond_data;
        std::string status_line;
        std::string respond_body;
        std::string host_directory;
        int status;

    public:
        respond_get(request_data *input, std::string host_directory);
        void read_file();
        std::string build_respond_data();
};

class respond_post
{
    private:
        request_data *request_info;
        std::string respond_data;
        std::string status_line;
        std::string respond_body;
        std::string host_directory;
        int status;

    public:
        respond_get(request_data *input, std::string host_directory);
        void read_file();
        std::string build_respond_data();
};

//char    *ft_itoa(int a);