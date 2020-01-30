
#include "purescript.h"
#include <list>
#include <glibmm.h>
#include <giomm.h>
#include <sigc++/trackable.h>
#include <cstring>
#include <gio/gnetworking.h>
#include <glib-unix.h>

using namespace purescript;

namespace {

  class NonCopyable {
  public:
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;  
  protected:
    NonCopyable();
  };
  inline NonCopyable::NonCopyable() {}

  Glib::ustring
  format_address(const Glib::RefPtr<Gio::SocketAddress> &address) {
    const Glib::RefPtr<Gio::InetSocketAddress> &inet_address =
      Glib::RefPtr<Gio::InetSocketAddress>::cast_dynamic(address);
    if (inet_address) {
      return Glib::ustring::compose(u8"%1:%2", inet_address->get_address()->to_string(), inet_address->get_port());
    } else {
      return u8"<Unknown Address>";
    }
  }
  
  class Server;

  class Socket: public NonCopyable, public sigc::trackable
  {
  public:
    Socket(std::list< std::shared_ptr< Socket> >&,
	   const std::function< void ( std::weak_ptr<Socket> socket ) >& _handleRead,
	   const Glib::RefPtr<Gio::SocketConnection>&);
    ~Socket();
    void set_iterator(std::list<std::shared_ptr< Socket> >::iterator);
    void close();
    void write( const std::string& data );    
    std::string read();
    
  private:
    void start_read();
    void finished_read(Glib::RefPtr<Gio::AsyncResult>&);
    void finished_write(Glib::RefPtr<Gio::AsyncResult>&);
    std::list< std::shared_ptr< Socket> >& connections;
    Glib::RefPtr<Gio::SocketConnection> sock;
    sigc::signal<void, std::weak_ptr< Socket> > signal_onRead;
    std::list< std::shared_ptr< Socket> >::iterator myself;
    std::string read_message;
    std::string write_message;
    char buffer[128] = { 0 };
  };
  
  class Server: public NonCopyable, public sigc::trackable
  {
  public:
    Server();
    ~Server();
    void setPort( int port );
    void close();
    void listen();

    template <class F> void onAccept( F&& f ) {
      _handleAccept = f;
      signal_onAccept.connect( _handleAccept);
    }
    template <class F> void onRead( F&& f ) { _handleRead = f; }
    void close( int fileDescriptor );
  private:
    bool on_incoming(const Glib::RefPtr<Gio::SocketConnection> &sock, const Glib::RefPtr<Glib::Object> &);
    std::function< void ( std::weak_ptr<Socket> socket ) > _handleAccept;
    std::function< void ( std::weak_ptr<Socket> socket ) > _handleRead;
    std::list< std::shared_ptr< Socket> > connections;
    Glib::RefPtr<Gio::SocketService> listener;
    sigc::signal<void, std::weak_ptr< Socket> > signal_onAccept;
  };

  Server::Server() :
    listener( Gio::SocketService::create())
  { listener->signal_incoming().connect(sigc::mem_fun(this, &Server::on_incoming)); }

  Server::~Server() { this->close(); }

  void Server::setPort( int port )
  { listener->add_inet_port( port); }

  void Server::close()
  {
    listener->stop();
    listener->close();
  }
  
  void Server::listen()
  { listener->start(); }

  bool Server::on_incoming( const Glib::RefPtr<Gio::SocketConnection> &sock,
			    const Glib::RefPtr<Glib::Object> &)
  {
    connections.emplace_back( std::make_shared<Socket>( connections, _handleRead, sock));
    std::list<std::shared_ptr< Socket> >::iterator iterator = connections.end();
    --iterator;
    (*iterator)->set_iterator(iterator);
    signal_onAccept.emit( *iterator);
    return false;
  }
  
  Socket::Socket(std::list< std::shared_ptr< Socket> >& connections_,
		 const std::function< void ( std::weak_ptr<Socket> socket ) >& handleRead,
		 const Glib::RefPtr<Gio::SocketConnection> &sock_ )
    : connections( connections_), sock( sock_)
  {
    if( handleRead) signal_onRead.connect( handleRead);
    if(!sock->get_socket()->set_option(IPPROTO_TCP, TCP_NODELAY, 1)) {
      //std::cout << "Warning: unable to set TCP_NODELAY option";
    }
    start_read();
  }
  Socket::~Socket() { sock->close(); }

  void Socket::set_iterator(std::list<std::shared_ptr< Socket> >::iterator iter)
  { myself = iter; }

  void Socket::close()
  { connections.erase( myself); }

  void Socket::start_read() {
    sock->get_input_stream()->read_async
      ( buffer, sizeof( buffer) - 1, sigc::mem_fun(this, &Socket::finished_read));
  }

  void Socket::finished_read( Glib::RefPtr<Gio::AsyncResult> &result) {
    try {
      auto bytes_read = sock->get_input_stream()->read_finish(result);
      if (bytes_read > 0 || !read_message.empty()) {
	buffer[bytes_read] = 0;
	read_message += buffer;
	if( ( bytes_read == sizeof( buffer) - 1) &&
	    buffer[bytes_read-1] != '\n' )
	  start_read();
	else
	  signal_onRead.emit( *myself);
      } else {
	close();
      }
    } catch (const Gio::Error &) {
      close();
    }
  }

  std::string Socket::read()
  {
    std::string message = read_message;
    read_message.clear();
    return message;
  }

  void Socket::write( const std::string& data )
  {
    write_message = data;
    sock->get_output_stream()->write_all_async
      ( reinterpret_cast<const void*>( write_message.c_str()), write_message.size(),
	sigc::mem_fun(this, &Socket::finished_write));
  } 
    
  void Socket::finished_write(Glib::RefPtr<Gio::AsyncResult> &result)
  {
    try {
      gsize bytes_written;
      if( sock->get_output_stream()->write_all_finish(result, bytes_written)) {
	if( bytes_written > 0) {
	  if( write_message.size() > bytes_written ) {
	    write( write_message.substr( bytes_written ));
	  } else {
	    write_message.clear();
	    start_read();
	  }
	} else {
	  close();
	}
      } else {
	close();
      }
    } catch (const Gio::Error &) {
      close();
    }
  }
}

extern "C" auto PS_Network_GioTcpServer_write() -> const boxed& {
  static const boxed _ = [](const boxed& socket_) -> boxed {
    return [=](const boxed& s_) -> boxed { // string
      return [=]() -> boxed { // effect
	auto& socket = const_cast< Socket&>(unbox<Socket>( socket_));
	const auto& s = unbox<std::string>( s_);
	socket.write( s);
	return boxed();
      };
    };
  };
  return _;
};

extern "C" auto PS_Network_GioTcpServer_read() -> const boxed& {
  static const boxed _ = [](const boxed& socket_) -> boxed {
    return [=]() -> boxed { // effect
      auto& socket = const_cast< Socket&>( unbox<Socket>( socket_));
      return boxed( socket.read());
    };
  };
  return _;
};

extern "C" auto PS_Network_GioTcpServer_close() -> const boxed& {
  static const boxed _ = [](const boxed& socket_) -> boxed {
    return [=]() -> boxed { // effect
      auto& socket = const_cast< Socket&>( unbox<Socket>( socket_));
      socket.close();
      return boxed();
    };
  };
  return _;
};

extern "C" auto PS_Network_GioTcpServer_createServer() -> const boxed& {
  static const boxed _ = [](const boxed& port_) -> boxed {
    return [=]() -> boxed { // effect
      const auto& port = unbox<int>( port_);
      auto server = std::make_shared<Server>();
      server->setPort( port);
      return boxed( server);
    };
  };
  return _;
};

extern "C" auto PS_Network_GioTcpServer_onRead() -> const boxed& {
  static const boxed _ = [](const boxed& server_) -> boxed {
    return [=](const boxed& f) -> boxed { //callback
      return [=]() -> boxed { // effect
	// we need a const cast here because the Server
	// will be modified...
	auto& server = const_cast<Server&>( unbox< Server>( server_));
	server.onRead( [=] ( std::weak_ptr<Socket> socket )
	{
	  if( auto s = socket.lock() ) {
	    f( s)();
	  }
	});
	return boxed();
      };
    };
  };
  return _;
};

extern "C" auto PS_Network_GioTcpServer_listen() -> const boxed& {
  static const boxed _ = [](const boxed& server_) -> boxed {
    return [=]() -> boxed { // effect
      auto& server = const_cast<Server&>( unbox< Server>( server_));
      server.listen();
      return boxed();
    };
  };
  return _;
};

