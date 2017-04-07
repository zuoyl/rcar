#include "json/writer.h"
#include <string>
#include <string.h>
#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include "message.h"

using namespace rcar;
using namespace std;

enum {
    MessageTypePushToAll,
    MessageTypePushToSingle,
    MessageTypePushToBatch,
    MessageTypePushToGroup,
    MessageTypeCreateGroup,
    MessageTypeDeleteGroup,
    MessageTypeAddToGroup,
    MessageTypeDeleteFromGroup,
    MessageTypeMax
};



#define AMQP_DEFAULT_PORT 5672


static void pushMessage(const Json::Value &msg) {
    const char *hostname = "localhost";
    int port = AMQP_DEFAULT_PORT;
    const char *exchange = "amq.direct";
    const char *routingkey = "rcar";

    Json::StyledWriter writer;
    std::string content = writer.write(msg);
 
    amqp_connection_state_t conn = amqp_new_connection();
    amqp_socket_t *socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        return;
    }

    int status = amqp_socket_open(socket, hostname, port);
    if (status) {
        return;
    }

    amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    amqp_channel_open(conn, 1);
    amqp_get_rpc_reply(conn);

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("application/json");
    props.delivery_mode = 2;
    amqp_basic_publish(conn, 1,
                      amqp_cstring_bytes(exchange),
                      amqp_cstring_bytes(routingkey),
                      0,
                      0,
                      &props,
                      amqp_cstring_bytes(content.c_str()));

    amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(conn);
}
void rcar::pushMessageToUsers(const vector<string> &ids, const Json::Value &msg) {
    if (ids.empty() || msg.empty()) {
        return;
    }
    Json::Value val;
    Json::Value targets(Json::arrayValue);
    for (vector<string>::const_iterator i = ids.begin(); i != ids.end(); i++)
        targets.append(*i);

    val["method"] = MessageTypePushToBatch;
    val["to"] = "user";
    val["targets"] = targets;
    val["msg"] = msg;
    pushMessage(val);
}


void rcar::pushMessageToUser(const std:: string &id, const Json::Value &msg) {
   if (id.empty() || msg.empty()) {
        return;
    }
   Json::Value val;
    val["method"] = MessageTypePushToSingle;
    val["to"] = "user";
    val["target"] = id;
    val["msg"] = msg;
    pushMessage(val);

}

void rcar::pushMessageToSellers(const std::vector<std::string> &ids, 
        const Json::Value &msg) {
    Json::Value val;
    Json::Value targets(Json::arrayValue);
    for (std::vector<std::string>::const_iterator i = ids.begin(); i != ids.end(); i++)
        targets.append(*i);

    val["method"] = MessageTypePushToBatch;
    val["to"] = "seller";
    val["targets"] = targets;
    val["msg"] = msg;
    pushMessage(val);
}

void rcar::pushMessageToSeller(const std::string &id, const Json::Value &msg) {
    if (id.empty() || msg.empty()) {
        return;
    }
    Json::Value val;
    val["method"] = MessageTypePushToSingle;
    val["to"] = "seller";
    val["target"] = id;
    val["msg"] = msg;
    pushMessage(val);
}

void rcar::pushMessageToGroup( const std::string& group_id, const Json::Value& msg) {}
void rcar::pushMessageToAllSellers(const Json::Value& msg) {}
void rcar::pushMessageToAllUsers(const Json::Value& msg) {}
void rcar::createGroup(const std::string &name) {}
void rcar::deleteGroup(const std::string &name) {}
void rcar::createGroupWithUsers(const std::string &name, const Json::Value &users){}
void rcar::deleteUserFromGroup(const std::string &name, const Json::Value &users){}
