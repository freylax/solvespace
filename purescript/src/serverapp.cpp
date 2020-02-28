#include <glibmm.h>
#include <giomm.h>
#include "ServerApp.h"
#include "Network_GioTcpServer.h"

int main(int argc, const char* argv[]) {

  Glib::init();
  Gio::init();

  auto loop = Glib::MainLoop::create();

  boxed v = PS_Network_GioTcpServer_createServer()(1031)();
  
  PS_ServerApp_main()(boxed(0))(v)();

  loop->run();

  return 0;
}
