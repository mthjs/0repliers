#define CATCH_CONFIG_MAIN

#include <0repliers.hpp>

#include <catch2/catch.hpp>

#include <zmq.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

struct Client {
   Client(std::string address)
      : __address(address)
   {
      __context = zmq_ctx_new();
      if (!__context)
         throw std::runtime_error(std::string("Client::Client: ") + zmq_strerror(zmq_errno()));
      __socket = zmq_socket(__context, ZMQ_REQ);
      if (!__socket)
         throw std::runtime_error(std::string("Client::Client: ") + zmq_strerror(zmq_errno()));
      if (zmq_connect(__socket, __address.c_str()) == -1)
         throw std::runtime_error(std::string("Client::Client: ") + zmq_strerror(zmq_errno()));
   }

   ~Client() {
      zmq_disconnect(__socket, __address.c_str());
      zmq_close(__socket);
      zmq_ctx_destroy(__context);
   }

   void send(const std::string &str) {
      if (zmq_send(__socket, str.c_str(), str.length(), 0) == -1)
         throw std::runtime_error(zmq_strerror(zmq_errno()));
   }

   std::string receive() {
      zmq_msg_t frame;
      zmq_msg_init(&frame);
      int received = zmq_msg_recv(&frame, __socket, 0);
      if (received == -1)
         throw std::runtime_error(std::string("Client::receive: unable to receive - ") + zmq_strerror(zmq_errno()));
      if (received == 0)
         return std::string("");
      std::string str(static_cast<const char*>(zmq_msg_data(&frame)), zmq_msg_size(&frame));
      zmq_msg_close(&frame);
      return str;
   }

private:
   void* __context;
   void* __socket;
   std::string __address;
};

TEST_CASE("a listener accepts new requests", "[Listener]") {
   SECTION("and returns a connection object from which we can read") {
      const std::string addr = "tcp://0.0.0.0:9876";
      const std::string msg = "hello, you!";

      std::thread listener([addr, msg](){
         auto listener = listen(addr);
         auto request = listener->accept();
         REQUIRE(request->read() == msg);
      });

      std::thread client([addr, msg](){
         Client client(addr);
         client.send(msg);
      });

      listener.join();
      client.join();
   }

   SECTION("which only can be replied to once") {
      const std::string addr = "tcp://0.0.0.0:8765";

      std::thread listener([addr](){
         auto listener = listen(addr);
         auto request = listener->accept();
         request->reply(request->read());
         REQUIRE_THROWS_AS(request->reply("replying twice is illegal"), std::logic_error);
      });

      std::thread client([addr](){
         Client client(addr);
         client.send("ohai Mark");
         REQUIRE(client.receive() == "ohai Mark");
      });

      listener.join();
      client.join();
   }

   SECTION("even if they come from the same client") {
      const std::string addr = "tcp://0.0.0.0:7654";

      std::thread listener([addr](){
         auto listener = listen(addr);
         for (;;) {
            std::thread worker([](std::unique_ptr<Request>&& request){
               request->reply(request->read());
            }, listener->accept());
            worker.detach();
         }
      });
      listener.detach();

      Client client(addr);
      client.send("Huey");
      REQUIRE(client.receive() == "Huey");
      client.send("Dewey");
      REQUIRE(client.receive() == "Dewey");
      client.send("Louie");
      REQUIRE(client.receive() == "Louie");
   }

   SECTION("and allows to handle them concurrently") {
      const std::string addr = "tcp://0.0.0.0:6543";

      std::thread listener([addr](){
         auto listener = listen(addr);
         for (;;) {
            std::thread worker([](std::unique_ptr<Request>&& request){
               request->reply(request->read());
            }, listener->accept());
            worker.detach();
         }
      });
      listener.detach();

      std::thread huey([addr](){
         Client client(addr);
         client.send("Huey");
         REQUIRE(client.receive() == "Huey");
      });

      std::thread dewey([addr](){
         Client client(addr);
         client.send("Dewey");
         REQUIRE(client.receive() == "Dewey");
      });

      std::thread louie([addr](){
         Client client(addr);
         client.send("Louie");
         REQUIRE(client.receive() == "Louie");
      });

      huey.join();
      dewey.join();
      louie.join();
   }
}

