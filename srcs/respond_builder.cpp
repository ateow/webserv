#include "../includes/respond_builder.hpp"
#include <ctime>

respond_builder::respond_builder(request_data *input, std::string host_directory) : request_info(input)
{
    std::cout << "\n>>>>> Generating http respond <<<<<<" << std::endl;

    this->server = "42_webserv";
    this->connection = input->get_connection();

    if (input->get_status_line() == 400)
        this->build_400_respond();
    else if (input->get_status_line() == 404)
        this->build_404_respond();

    // this->status = 200;
    // this->status_line = "HTTP/1.1 200 OK";
    
    // // Body
    // this->host_directory = host_directory;
    // this->read_file();
}

void respond_builder::build_400_respond()
{
    std::ifstream file;
    std::ostringstream ss;

    this->status = 400;
    this->status_line = "HTTP/1.1 400 Bad Request";
    this->content_type = "text/html";
    file.open("../errors/400.html"); // need to path depending on where main is called!
	ss << file.rdbuf();
    this->respond_body = ss.str();
    this->content_length = this->respond_body.length();
}

void respond_builder::build_404_respond()
{
    std::ifstream file;
    std::ostringstream ss;

    this->status = 404;
    this->status_line = "HTTP/1.1 404 Not Found";
    this->content_type = "text/html";
    file.open("../errors/404.html"); // need to path depending on where main is called!
	ss << file.rdbuf();
    this->respond_body = ss.str();
    this->content_length = this->respond_body.length();
}


void respond_builder::read_file()
{
    std::ifstream file((host_directory + request_info->get_target()).c_str());

    if (file.fail())
    {
        this->status = 404;
        this->status_line = "HTTP/1.1 404 Not Found";
        file.open("../errors/404.html"); // need to path depending on where main is called!
    }
    std::ostringstream ss;
	ss << file.rdbuf();
    this->respond_body = ss.str();
}

std::string respond_builder::build_respond_data()
{
    std::string respond_txt;
    // get current time
    std::time_t currentTime = std::time(NULL);
    std::tm* localTime = std::gmtime(&currentTime); // Convert to GMT time
    // Format the date string
    char dateString[128];
    std::strftime(dateString, sizeof(dateString), "%a, %d %b %Y %H:%M:%S GMT", localTime);

    std::stringstream ss;
    ss << this->content_length;
    respond_txt = this->status_line + "\r\n"
                    + "Server: " + this->server + "\r\n"
                    + "Date: " + dateString + "\r\n"
                    + "Content-Type: " + this->content_type + "\r\n"
                    + "Content-Length: " + ss.str() + "\r\n"
                    + "Connection: " + this->connection + "\r\n\r\n"
                    + this->respond_body;
    this->respond_data = respond_txt;
    return(this->respond_data);
}

int main()
{
    std::string host_directory = "../";
    std::string request_details_from_browser;
    request_details_from_browser = "GEsT /index.html../../../ HTTP/1.1\r\nHost: 127.0.0.1:80\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:97.0) Gecko/20100101 Firefox/97.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate, br\r\nConnection: keep-alive\r\n\r\n";
    //request_details_from_browser = "POST /cgi-bin/process_form.cgi HTTP/1.1\r\nHost: 127.0.0.1:80\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 33\r\n\r\nusername=johndoe&password=secret\r\n";
    
    request_data *input = new request_data(request_details_from_browser);
    std::cout << input->get_method() << std::endl;
    std::cout << input->get_target() << std::endl;
    std::cout << input->get_http_version() << std::endl;
    std::cout << input->get_host() << std::endl;
    std::cout << input->get_port() << std::endl;
    std::cout << input->get_user_agent() << std::endl;
    std::cout << input->get_accept_language() << std::endl;
    std::cout << input->get_accept_encoding() << std::endl;
    std::cout << input->get_connection() << std::endl;
    std::cout << input->get_content_type() << std::endl;
    std::cout << input->get_content_length() << std::endl;
    std::cout << input->get_body() << std::endl;
    std::cout << input->get_status_line() << std::endl;

    respond_builder *output = new respond_builder(input, host_directory);
    std::cout << output->build_respond_data() << std::endl;
}