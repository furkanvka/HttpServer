#ifndef CLI_H
#define CLI_H

#include <string>
#include "core/http_server.h"

namespace cli {
    void print_help();
    void print_version();
    void create_server(int port);
    
    int parse_arguments(int argc, char* argv[]);
}

#endif
