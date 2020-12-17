#ifndef _0REPLIERS_LISTENER
#define _0REPLIERS_LISTENER

#include <memory>
#include <string>

struct Writer
{
   virtual void write(const std::string &identity, const std::string &payload) = 0;
};

struct Router : Writer
{
   virtual void read(std::string& identiy, std::string& payload) = 0;
};

/**
 * Request is a request that Listener has accepted.
 */
struct Request
{
   /**
    * read will copy the entire content of the request. Calling read multiple
    * times is allowed and will always return the same value
    */
   virtual std::string read() const noexcept = 0;

   /**
    * reply will send the given answer to the requesting client. It is not
    * allowed to reply multiple times. Doing so will result in a
    *`std::logic_error` being thrown.
    */
   virtual void reply(const std::string &answer) = 0;
};

/**
 * Listener listens for new requests to accept.
 */
struct Listener
{
   /**
    * accept creates a new connection object from the first available message.
    */
   virtual std::unique_ptr<Request> accept() = 0;
};

/**
 * listen creates a new listener which'll start listening on the given address.
 */
std::unique_ptr<Listener> listen(std::string address);

std::shared_ptr<Router> route(std::string address);

#endif
