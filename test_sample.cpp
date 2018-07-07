#include "raft.h"

int main(int argc, char *argv[]) {
    std::map<std::string, std::string>config;
    config["listen_node"] = argv[1]; 
    init_raft(config);
}
