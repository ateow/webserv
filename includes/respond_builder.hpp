#include <iostream>
#include <sstream>
#include <fstream> 
#include "../includes/request_parser.hpp"

class respond_builder
{
    private:
        request_data *request_info;
        std::string respond_data;
        std::string status_line;
        std::string respond_body;
        std::string host_directory;
        int status;
        std::string server;
        std::string date;
        std::string content_type;
        int content_length;
        std::string connection;

// Server: nginx/1.24.0
// Date: Sat, 27 Apr 2024 15:20:52 GMT
// Content-Type: text/html
// Content-Length: 153
// Connection: keep-alive


    public:
        respond_builder(request_data *input, std::string host_directory);
        void read_file();
        void build_400_respond();
        void build_404_respond();
        std::string build_respond_data();
};

// class respond_post
// {
//     private:
//         request_data *request_info;
//         std::string respond_data;
//         std::string status_line;
//         std::string respond_body;
//         std::string host_directory;
//         int status;

//     public:
//         respond_post(request_data *input, std::string host_directory);
//         void read_file();
//         std::string build_respond_data();
// };
