#include "../includes/request_parser.hpp"

request_data::request_data(std::string input, std::string host_directory, std::string cgi_directory) : request_text(input)
{
    std::cout << ">>>>> Parsing HTTP request <<<<<" << std::endl;
    this->status_line = 200;
    this->content_length = 0;
    this->parse_method();
    this->parse_target(host_directory, cgi_directory);
    this->parse_version();
    this->parse_headers();
}

int request_data::parse_method()
{
    size_t first_space_pos = request_text.find(' ');
    if (first_space_pos <= std::string::npos)
    {
        std::string tmp = request_text.substr(0,first_space_pos);
        if (tmp != "GET" && tmp != "POST" && tmp != "DELETE")
        {
            int i = 0;
            while (tmp[i] != '\0')
            {
                if (std::isupper(tmp[i]) == 0) 
                {
                    this->status_line = 400;
                    this->method = "Invalid";
                    return (1);
                }
                i++;
            }
            this->status_line = 405;
            this->method = "Invalid";
        }
        this->method = tmp;
        return (0);
    }
}

int request_data::parse_target(std::string host_directory, std::string cgi_directory)
{
    size_t first_space_pos = request_text.find(' ');
    if (first_space_pos != std::string::npos) 
    {
        size_t second_space_pos = request_text.find(' ', first_space_pos + 1);
        if (second_space_pos != std::string::npos) 
        {
            std::string line = request_text.substr(first_space_pos + 1, second_space_pos - first_space_pos - 1);
            
            // ERROR Check:

            // (1) check for long URI
            if (line.length() >= 2000)
            {
                if (this->status_line == 200)
                    this->status_line = 414;
                this->target = "URI Too long";
            }
            
            // (2) check for URI injection and access
            int depth = 0;
            size_t pos = 0;
            while ((pos = line.find("../", pos)) != std::string::npos) 
            {
                if (pos == 0 || line[pos - 1] == '/')
                    depth--;
                pos += 3;
            }
            pos = 0;
            while ((pos = line.find("/", pos)) != std::string::npos) 
            {
                if (pos > 0 && line[pos - 1] != '/' && line[pos - 1] != '.')
                    depth++; // Increment depth for each occurrence of "/"
                pos++; 
            }
            if (depth < 0)
                this->target = "/";

            // (3) check Valid Resource if not CGI
            if (line.substr(0, 9) != "/cgi-bin/")
            {
                std::ifstream file((host_directory + line).c_str());
                if (file.fail() && this->status_line == 200)
                    this->status_line = 404;
                this->cgi_bin = "no";
                this->target = host_directory + line;
            }
            else if (line.substr(0, 9) == "/cgi-bin/")
            {
                // check if cgi exist
                // check if cgi workable
                // update target if no issue for respond to build
                std::ifstream file((cgi_directory + line.substr(9)).c_str());
                if (file.fail() && this->status_line == 200)
                    this->status_line = 404;
                this->target = cgi_directory + line.substr(9);
                this->cgi_bin = "yes";
            }
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
    while (requesttxt != "\0")
    {
        //std::cout << "loop:\n" << requesttxt << std::endl;
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
        else if (line.substr(0, line.find(' ')) == "User-Agent:")
        {
            this->user_agent = line.substr(line.find(' ') + 1);
        }
        // Accept
        else if (line.substr(0, line.find(' ')) == "Accept:")
        {
            this->accept = line.substr(line.find(' ') + 1);
        }
        // Accept-Language
        else if (line.substr(0, line.find(' ')) == "Accept-Language:")
        {
            this->accept_language = line.substr(line.find(' ') + 1);
        }
        // Accept-Encoding
        else if (line.substr(0, line.find(' ')) == "Accept-Encoding:")
        {
            this->accept_encoding = line.substr(line.find(' ') + 1);
        }
        // Connection
        else if (line.substr(0, line.find(' ')) == "Connection:")
        {
            this->connection = line.substr(line.find(' ') + 1);
        }
        // Content-Length
        else if (line.substr(0, line.find(' ')) == "Content-Length:")
        {
            this->content_length = ft_atoi(line.substr(line.find(' ') + 1).c_str());
        }
        // Content-type
        else if (line.substr(0, line.find(' ')) == "Content-Type:")
        {
            this->content_type = line.substr(line.find(' ') + 1);
        }
        // Body
        else if (this->content_length > 0 && line.length() != 0)
        {
            this->body = line;
        }
    }
    return (0);
}

std::string request_data::get_method()
{
    return(this->method);
}

std::string request_data::get_target()
{
    return(this->target);
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

int request_data::get_content_length()
{
    return(this->content_length);
}

std::string request_data::get_content_type()
{
    return(this->content_type);
}
std::string request_data::get_body()
{
    return(this->body);
}

int request_data::get_status_line()
{
    return(this->status_line);
}

std::string request_data::get_cgi_bin()
{
    return(this->cgi_bin);
}
// int main()
// {
//     std::string request_header;
//     request_header = "GET /index.html HTTP/1.1\r\nHost: www.example.com:8080\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) Gecko/20100101 Firefox/97.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";

//     request_data *input = new request_data(request_header);
//     std::cout << input->get_method() << std::endl;
//     std::cout << input->get_target() << std::endl;
//     std::cout << input->get_http_version() << std::endl;
//     std::cout << input->get_host() << std::endl;
//     std::cout << input->get_port() << std::endl;
//     std::cout << input->get_user_agent() << std::endl;
//     std::cout << input->get_accept_language() << std::endl;
//     std::cout << input->get_accept_encoding() << std::endl;
//     std::cout << input->get_connection() << std::endl;
// }