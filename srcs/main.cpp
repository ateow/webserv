#include "libs.hpp"
#include "EpollServer.hpp"

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config-file>" << std::endl;
        return 1;
    }

    WebServerConfig config;
    config.parseConfig(argv[1]);  // Parse the config file specified by the command line
	std::cout << config.servers.size() << std::endl;
	EpollServer webserv(config);

	webserv.runServer();
	
	return 0;
}

// int main(int argc, char** argv) {


//     // Print parsed data for verification
//     for (size_t i = 0; i < config.servers.size(); ++i) {
//         const ServerConfig& server = config.servers[i];
//         std::cout << "Server " << i + 1 << ":\n";
//         std::cout << "  Host: " << server.host << "\n";
//         std::cout << "  Port: " << server.port << "\n";
//         std::cout << "  Server Names: ";
//         for (size_t j = 0; j < server.server_names.size(); ++j) {
//             std::cout << server.server_names[j] << (j + 1 < server.server_names.size() ? ", " : "\n");
//         }
//         std::cout << "  Client Body Size Limit: " << server.limit_client_body_size << "\n";
//         std::cout << "  Default Error Pages:\n";
//         for (std::map<int, std::string>::const_iterator it = server.default_error_pages.begin();
//              it != server.default_error_pages.end(); ++it) {
//             std::cout << "    " << it->first << ": " << it->second << "\n";
//         }

//         // Print Route Configurations
//         for (size_t j = 0; j < server.routes.size(); ++j) {
//             const RouteConfig& route = server.routes[j];
//             std::cout << "  Route " << j + 1 << ":\n";
//             std::cout << "    Path: " << route.path << "\n";
//             std::cout << "    Root Directory: " << route.root_directory << "\n";
//             std::cout << "    Default File: " << route.default_file << "\n";
//             std::cout << "    Directory Listing: " << (route.list_directory ? "On" : "Off") << "\n";
//             std::cout << "    Accepted Methods: ";
//             for (size_t k = 0; k < route.accepted_methods.size(); ++k) {
//                 std::cout << route.accepted_methods[k] << (k + 1 < route.accepted_methods.size() ? ", " : "\n");
//             }
//             std::cout << "    Redirect: " << route.redirect << "\n";
//             std::cout << "    CGI Enabled: " << (route.cgi_enable ? "Yes" : "No") << "\n";
//             std::cout << "    CGI Path: " << route.cgi_path << "\n";
//             std::cout << "    CGI Extensions: " << route.cgi_extensions << "\n";
//             std::cout << "    Upload Enabled: " << (route.upload_enable ? "Yes" : "No") << "\n";
//             std::cout << "    Upload Path: " << route.upload_path << "\n";
//         }
//     }

//     // Print CGI Global Configuration
//     std::cout << "Global CGI Configuration:\n";
//     std::cout << "  CGI Bin Path: " << config.cgi_config.cgi_bin_path << "\n";
//     std::cout << "  PHP CGI: " << config.cgi_config.php_cgi << "\n";
//     std::cout << "  Python CGI: " << config.cgi_config.python_cgi << "\n";
//     std::cout << "  Executable Extensions: " << config.cgi_config.cgi_executable_extensions << "\n";
//     std::cout << "  Chunk Handling: " << config.chunk_handling << "\n";

//     return 0;
// }
