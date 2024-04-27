#include "../includes/respond_builder.hpp"

respond_get::respond_get(request_data *input, std::string host_directory) : request_info(input)
{
    std::cout << "\nGenerating http respond..." << std::endl;
    this->status = 200;
    this->status_line = "HTTP/1.1 200 OK";
    
    // Body
    this->host_directory = host_directory;
    this->read_file();
}

void respond_get::read_file()
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

std::string respond_get::build_respond_data()
{
    std::string respond_txt;
    respond_txt = this->status_line + "\r\n\r\n" + this->respond_body;
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

    if (input->get_status_line() != 200)
    {
        std::cout << "create error response" << std::endl;
    }
    else if (input->get_method() == "GET")
    {
        respond_get *output = new respond_get(input, host_directory);
        std::cout << output->build_respond_data() << std::endl;
    }
    // else if (input->get_method() == "POST")
    // {
    //     respond_get *output = new respond_post(input, host_directory);
    //     std::cout << output->build_respond_data() << std::endl;
    // }
}