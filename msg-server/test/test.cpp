#include "stdio.h"
#include "message.h"


void testUser() {
    std::string user = "13889536307";
    Json::Value msg;
    msg["source"] = "user";
    rcar::pushMessageToSeller(user, msg);
}

void testSeller() {
    std::string user = "13889536307";
    Json::Value msg;
    msg["source"] = "seller";
    msg["kind"] = "msg";
    rcar::pushMessageToUser(user, msg);
}

void testAll() {

}

void usage() {
    std::cout << "usage: test [user|seller|all]" << std::endl;
}

int main(int argc, char const* const* argv) {
    if (argc < 2) {
        usage();
        return 0;
    }
    std::string target = argv[1];
    if (target == "user")
        testUser();
    else if (target == "seller")
        testSeller();
    else if (target == "all") 
        testAll();
    else 
        usage();
   return 0;
}
