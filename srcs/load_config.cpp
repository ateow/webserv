#include "load_config.hpp"

void WebServerConfig::parseConfig(const std::string& filename) {
    std::ifstream config_file(filename.c_str());
    std::string line;
    ServerConfig *current_server = NULL;
    RouteConfig *current_route = NULL;

    if (!config_file.is_open()) {
        std::cerr << "Failed to open configuration file." << std::endl;
        return;
    }

    while (getline(config_file, line)) {
        trim(line);
        if (line.empty() || line[0] == '#') continue; // Skip comments and empty lines

        if (line[0] == '[') {
            // New section starts
            if (line.find("[server") != std::string::npos) {
                ServerConfig new_server;
                servers.push_back(new_server);
                current_server = &servers.back();
            } else if (line.find("[route") != std::string::npos) {
                if (current_server) {
                    RouteConfig new_route;
                    current_server->routes.push_back(new_route);
                    current_route = &current_server->routes.back();
                }
            } else if (line.find("[cgi_global]") != std::string::npos) {
                current_route = NULL;  // No current route for global CGI config
            }
            continue;
        }

        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            std::getline(is_line, value);
            trim(key);
            trim(value);

            // Parse the key-value pair
            if (current_route) {
                if (key == "path") current_route->path = value;
                else if (key == "root_directory") current_route->root_directory = value;
                else if (key == "default_file") current_route->default_file = value;
                else if (key == "list_directory") current_route->list_directory = (value == "on" || value == "true");
                else if (key == "accepted_methods") {
                    std::istringstream methods(value);
                    std::string method;
                    while (std::getline(methods, method, ' ')) {
                        trim(method);
                        if (!method.empty()) current_route->accepted_methods.push_back(method);
                    }
                }
                else if (key == "redirect") current_route->redirect = value;
                else if (key == "cgi_enable") current_route->cgi_enable = (value == "true" || value == "on");
                else if (key == "cgi_path") current_route->cgi_path = value;
                else if (key == "cgi_extensions") current_route->cgi_extensions = value;
                else if (key == "upload_enable") current_route->upload_enable = (value == "true" || value == "on");
                else if (key == "upload_path") current_route->upload_path = value;
            } else if (current_server) {
                if (key == "host") current_server->host = value;
                else if (key == "port") current_server->port = atoi(value.c_str());
                else if (key == "server_names") {
                    std::istringstream names(value);
                    std::string name;
                    while (std::getline(names, name, ' ')) {
                        trim(name);
                        if (!name.empty()) current_server->server_names.push_back(name);
                    }
                }
                else if (key == "default_error_pages") {
                    std::istringstream errors(value);
                    std::string error;
                    while (std::getline(errors, error, ' ')) {  // Assume spaces between entries
                        trim(error);
                        size_t colon_pos = error.find(':');
                        if (colon_pos != std::string::npos) {
                            int code = atoi(error.substr(0, colon_pos).c_str());
                            std::string path = error.substr(colon_pos + 1);
                            current_server->default_error_pages[code] = path;
                        }
                    }
                }
                else if (key == "limit_client_body_size") current_server->limit_client_body_size = value;
            } 
                // Global CGI settings
                if (key == "cgi_bin_path") cgi_config.cgi_bin_path = value;
                else if (key == "php_cgi") cgi_config.php_cgi = value;
                else if (key == "python_cgi") cgi_config.python_cgi = value;
                else if (key == "cgi_executable_extensions") cgi_config.cgi_executable_extensions = value;
                else if (key == "chunk_handling") chunk_handling = value;
        }
    }

    config_file.close();
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config-file>" << std::endl;
        return 1;
    }

    WebServerConfig config;
    config.parseConfig(argv[1]);  // Parse the config file specified by the command line

    // Print parsed data for verification
    for (size_t i = 0; i < config.servers.size(); ++i) {
        const ServerConfig& server = config.servers[i];
        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  Host: " << server.host << "\n";
        std::cout << "  Port: " << server.port << "\n";
        std::cout << "  Server Names: ";
        for (size_t j = 0; j < server.server_names.size(); ++j) {
            std::cout << server.server_names[j] << (j + 1 < server.server_names.size() ? ", " : "\n");
        }
        std::cout << "  Client Body Size Limit: " << server.limit_client_body_size << "\n";
        std::cout << "  Default Error Pages:\n";
        for (std::map<int, std::string>::const_iterator it = server.default_error_pages.begin();
             it != server.default_error_pages.end(); ++it) {
            std::cout << "    " << it->first << ": " << it->second << "\n";
        }

        // Print Route Configurations
        for (size_t j = 0; j < server.routes.size(); ++j) {
            const RouteConfig& route = server.routes[j];
            std::cout << "  Route " << j + 1 << ":\n";
            std::cout << "    Path: " << route.path << "\n";
            std::cout << "    Root Directory: " << route.root_directory << "\n";
            std::cout << "    Default File: " << route.default_file << "\n";
            std::cout << "    Directory Listing: " << (route.list_directory ? "On" : "Off") << "\n";
            std::cout << "    Accepted Methods: ";
            for (size_t k = 0; k < route.accepted_methods.size(); ++k) {
                std::cout << route.accepted_methods[k] << (k + 1 < route.accepted_methods.size() ? ", " : "\n");
            }
            std::cout << "    Redirect: " << route.redirect << "\n";
            std::cout << "    CGI Enabled: " << (route.cgi_enable ? "Yes" : "No") << "\n";
            std::cout << "    CGI Path: " << route.cgi_path << "\n";
            std::cout << "    CGI Extensions: " << route.cgi_extensions << "\n";
            std::cout << "    Upload Enabled: " << (route.upload_enable ? "Yes" : "No") << "\n";
            std::cout << "    Upload Path: " << route.upload_path << "\n";
        }
    }

    // Print CGI Global Configuration
    std::cout << "Global CGI Configuration:\n";
    std::cout << "  CGI Bin Path: " << config.cgi_config.cgi_bin_path << "\n";
    std::cout << "  PHP CGI: " << config.cgi_config.php_cgi << "\n";
    std::cout << "  Python CGI: " << config.cgi_config.python_cgi << "\n";
    std::cout << "  Executable Extensions: " << config.cgi_config.cgi_executable_extensions << "\n";
    std::cout << "  Chunk Handling: " << config.chunk_handling << "\n";

    return 0;
}
