#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <assert.h>
#include <unistd.h>

#include "utils.h"
#include <iostream>
#include "message-factory.h"
#include "lib/sys-config.h"
#include "lib/log.h"

#define SUMMARY_EVERY_US 1000000


using namespace rcar;

static void run(amqp_connection_state_t conn) {
  uint64_t start_time = now_microseconds();
  int received = 0;
  int previous_received = 0;
  uint64_t previous_report_time = start_time;
  uint64_t next_summary_time = start_time + SUMMARY_EVERY_US;

  amqp_frame_t frame;
  uint64_t now;


  while (1) {
    amqp_rpc_reply_t ret;
    amqp_envelope_t envelope;

    now = now_microseconds();
    if (now > next_summary_time) {
      int countOverInterval = received - previous_received;
      double intervalRate = countOverInterval / ((now - previous_report_time) / 1000000.0);
      printf("%d ms: Received %d - %d since last report (%d Hz)\n",
             (int)(now - start_time) / 1000, received, countOverInterval, (int) intervalRate);

      previous_received = received;
      previous_report_time = now;
      next_summary_time += SUMMARY_EVERY_US;
    }

    amqp_maybe_release_buffers(conn);
    ret = amqp_consume_message(conn, &envelope, NULL, 0);

    if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
        amqp_destroy_envelope(&envelope);
        continue;
    }

    rcar::MessageFactory *factory = rcar::MessageFactory::getInstance();
    factory->handleMessage((const char*)envelope.message.body.bytes, envelope.message.body.len);
    amqp_destroy_envelope(&envelope);
    received++;
  }
}

int main(int argc,  char **argv) {
    // initialize ios push cert
  rcar::SystemConfig *config = rcar::SystemConfig::getInstance();
  if (!config->create(argc, argv, "rcar-msg-server")) {
      std::cout << "failed to create configuration" << std::endl;
  }

  // initialize cert with server
  rcar::MessageFactory *factory = rcar::MessageFactory::getInstance();
  if (!factory->initializeCert()) {
      std::cout << "failed to initialize cert" << std::endl;
  }
  // get server parameter
  std::string hostname = config->getGeneral("rabbitmq-server");
  int port = config->getPort();
  char const *exchange = "amq.direct"; /* argv[3]; */
  char const *bindingkey = "rcar"; /* argv[4]; */
  amqp_bytes_t queuename;

  amqp_connection_state_t conn = amqp_new_connection();
  amqp_socket_t *socket = amqp_tcp_socket_new(conn);
  if (!socket) {
    die("creating TCP socket");
  }

  int status = amqp_socket_open(socket, hostname.c_str(), port);
  if (status) {
    die("opening TCP socket");
  }

  die_on_amqp_error(amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest"),
                    "Logging in");
  amqp_channel_open(conn, 1);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Opening channel");

  {
    amqp_queue_declare_ok_t *r = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 0, 1,
                                 amqp_empty_table);
    die_on_amqp_error(amqp_get_rpc_reply(conn), "Declaring queue");
    queuename = amqp_bytes_malloc_dup(r->queue);
    if (queuename.bytes == NULL) {
        Log::hl_info("Out of memory while copying queue name");
        return 1;
    }
  }

  amqp_queue_bind(conn, 1, queuename, amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey),
                  amqp_empty_table);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Binding queue");

  amqp_basic_consume(conn, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
  die_on_amqp_error(amqp_get_rpc_reply(conn), "Consuming");


  // dameon
  daemon(0, 0);

  run(conn);

  die_on_amqp_error(amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
  die_on_error(amqp_destroy_connection(conn), "Ending connection");

  return 0;
}
