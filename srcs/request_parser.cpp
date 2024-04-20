#include "../includes/request_parser.hpp"

request_data::request_data(std::string input) : request_text(input)
{
 std::cout << "Initialising http request..." << std::endl;
    this->parse_method();
    this->parse_URL();
    this->parse_version();
    this->parse_headers();
}

int request_data::parse_method()
{
    size_t first_space_pos = request_text.find(' ');
    if (first_space_pos <= std::string::npos)
    {
        std::string tmp = request_text.substr(0,first_space_pos);
        if (tmp != "GET" && tmp != "POST")
        {
            return (1);
        }
        this->method = tmp;
        return (0);
    }
}

int request_data::parse_URL()
{
    size_t first_space_pos = request_text.find(' ');
    if (first_space_pos != std::string::npos) 
    {
        size_t second_space_pos = request_text.find(' ', first_space_pos + 1);
        if (second_space_pos != std::string::npos) 
        {
            this->URL = request_text.substr(first_space_pos + 1, second_space_pos - first_space_pos - 1);
        }
    }
    return (0);
}

int request_data::parse_version()
{
    size_t first_space_pos = request_text.find(' ');
    if (first_space_pos != std::string::npos) 
    {
        size_t second_space_pos = request_text.find(' ', first_space_pos + 1);
        if (second_space_pos != std::string::npos) 
        {
            size_t third_space_pos = request_text.find("\r\n");
            if (third_space_pos != std::string::npos) 
            {
                this->http_version = request_text.substr(second_space_pos + 1, third_space_pos - second_space_pos - 1);
            }
        }
    }
    return (0);
}

int request_data::parse_headers()
{
    std::string requesttxt = request_text;
    while (requesttxt.substr(0, 4) != "\r\n")
    {
        requesttxt = requesttxt.substr(requesttxt.find("\r\n") + 2);
        std::string line = requesttxt.substr(0, requesttxt.find("\r\n"));
        // Host and Port
        if (line.substr(0, line.find(' ')) == "Host:")
        {
            std::string host = line.substr(line.find(' ') + 1);
            if (host.find(':') == std::string::npos)
            {
                this->host = host;
                this->port = 80;
            }
            else
            {
                this->host = host.substr(0, host.find(':'));
                this->port = ft_atoi(host.substr(host.find(':') + 1).c_str());
            }
        }
        // User-Agent
        if (line.substr(0, line.find(' ')) == "User-Agent:")
        {
            this->user_agent = line.substr(line.find(' ') + 1);
        }
        // Accept
        if (line.substr(0, line.find(' ')) == "Accept:")
        {
            this->accept = line.substr(line.find(' ') + 1);
        }
        // Accept-Language
        if (line.substr(0, line.find(' ')) == "Accept-Language:")
        {
            this->accept_language = line.substr(line.find(' ') + 1);
        }
        // Accept-Encoding
        if (line.substr(0, line.find(' ')) == "Accept-Encoding:")
        {
            this->accept_encoding = line.substr(line.find(' ') + 1);
        }
        // Connection
        if (line.substr(0, line.find(' ')) == "Connection:")
        {
            this->connection = line.substr(line.find(' ') + 1);
        }
    }
    return (0);
}

std::string request_data::get_method()
{
    return(this->method);
}

std::string request_data::get_URL()
{
    return(this->URL);
}

std::string request_data::get_http_version()
{
    return(this->http_version);
}

std::string request_data::get_host()
{
    return(this->host);
}

int request_data::get_port()
{
    return(this->port);
}

std::string request_data::get_user_agent()
{
    return(this->user_agent);
}

std::string request_data::get_accept()
{
    return(this->accept);
}

std::string request_data::get_accept_language()
{
    return(this->accept_language);
}


std::string request_data::get_accept_encoding()
{
    return(this->accept_encoding);
}


std::string request_data::get_connection()
{
    return(this->connection);
}


int main()
{
    std::string request_header;
    request_header = "GET /index.html HTTP/1.1\r\nHost: www.example.com:8080\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) Gecko/20100101 Firefox/97.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";

    request_data *input = new request_data(request_header);
    std::cout << input->get_method() << std::endl;
    std::cout << input->get_URL() << std::endl;
    std::cout << input->get_http_version() << std::endl;
    std::cout << input->get_host() << std::endl;
    std::cout << input->get_port() << std::endl;
    std::cout << input->get_user_agent() << std::endl;
    std::cout << input->get_accept_language() << std::endl;
    std::cout << input->get_accept_encoding() << std::endl;
    std::cout << input->get_connection() << std::endl;
}