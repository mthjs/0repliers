#include <0repliers.hpp>

#include <zmq.h>

#include <exception>
#include <mutex>

struct Writer
{
   virtual void write(const std::string &identity, const std::string &payload) = 0;
};

struct Transport
   : Writer
{
   Transport(std::string address)
      : __address(address)
   {
      __context = zmq_ctx_new();
      if (!__context)
         throw std::runtime_error(std::string("Transport::Transport: ") + zmq_strerror(zmq_errno()));
      __socket = zmq_socket(__context, ZMQ_ROUTER);
      if (!__socket)
         throw std::runtime_error(std::string("Transport::Transport: ") + zmq_strerror(zmq_errno()));
      if (zmq_bind(__socket, __address.c_str()) == -1)
         throw std::runtime_error(std::string("Transport::Transport: ") + zmq_strerror(zmq_errno()));
   };

   ~Transport()
   {
      zmq_unbind(__socket, __address.c_str());
      zmq_close(__socket);
      zmq_ctx_destroy(__context);
   };

   void read(std::string &identity, std::string &payload)
   {
      zmq_pollitem_t items[1];
      items[0].socket = __socket;
      items[0].events = ZMQ_POLLIN;
      zmq_poll(items, 1, -1);
      std::lock_guard<std::mutex> lock(__lock);
      identity = __read_frame();
      __read_frame(); // delimiter frame.
      payload = __read_frame();
   };

   void write(const std::string &identity, const std::string &payload)
   {
      std::lock_guard<std::mutex> lock(__lock);
      if (zmq_send(__socket, identity.c_str(), identity.length(), ZMQ_SNDMORE) == -1)
         throw std::runtime_error(std::string("Connection::write: unable to send identity-frame - ") + zmq_strerror(zmq_errno()));
      if (zmq_send(__socket, "", 0, ZMQ_SNDMORE) == -1)
         throw std::runtime_error(std::string("Connection::write: unable to send delimiter-frame - ") + zmq_strerror(zmq_errno()));
      if (zmq_send(__socket, payload.c_str(), payload.length(), 0) == -1)
         throw std::runtime_error(std::string("Connection::write: unable to send the payload - ") + zmq_strerror(zmq_errno()));
   };

private:
   std::string __read_frame()
   {
      zmq_msg_t frame;
      zmq_msg_init(&frame);
      int received = zmq_msg_recv(&frame, __socket, 0);
      if (received == -1)
         throw std::runtime_error(std::string("Listener::__receive_frame: unable to receive - ") + zmq_strerror(zmq_errno()));
      if (received == 0)
         return std::string("");
      std::string str(static_cast<const char*>(zmq_msg_data(&frame)), zmq_msg_size(&frame));
      zmq_msg_close(&frame);
      return str;
   };

private:
   std::string __address;

   std::mutex __lock;

   void* __context;
   void* __socket;
};

struct ListenerImpl;

struct RequestImpl : Request
{
   friend ListenerImpl;

   std::string read() const noexcept
   {
      return __payload;
   };

   void reply(const std::string &answer)
   {
      __writer->write(__identity, answer);
   };

private:
   std::shared_ptr<Writer> __writer;
   std::string __identity;
   std::string __payload;
};

struct ListenerImpl
   : Listener
{
   ListenerImpl(std::string address)
      : __transport(std::make_shared<Transport>(address))
   {};

   std::unique_ptr<Request> accept()
   {
      std::unique_ptr<RequestImpl> request(new RequestImpl());
      __transport->read(request->__identity, request->__payload);
      request->__writer = __transport;
      return request;
   };

private:
   std::shared_ptr<Transport> __transport;
};

std::unique_ptr<Listener> listen(std::string address)
{
   return std::unique_ptr<Listener>(new ListenerImpl(address));
}
