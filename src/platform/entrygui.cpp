//-----------------------------------------------------------------------------
// Our main() function for the graphical interface.
//
// Copyright 2018 <whitequark@whitequark.org>
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include <iostream>
#if defined(WIN32)
#   include <windows.h>
#endif
#if defined(PS_ENABLED)
#   include <giomm.h>
#   include "Server.h"
#   include "Network_GioTcpServer.h"
#   include "Solvespace_Resources.h"
#endif

class Foo {
public:
  ~Foo() {
    std::cout << "~Foo" << std::endl;
  }
};

using namespace SolveSpace;

int main(int argc, char** argv) {
    std::vector<std::string> args = InitPlatform(argc, argv);

    Platform::InitGui(argc, argv);
    Platform::Open3DConnexion();
    SS.Init();

    if(args.size() >= 2) {
        if(args.size() > 2) {
            dbp("Only the first file passed on command line will be opened.");
        }

        SS.Load(Platform::Path::From(args.back()).Expand(/*fromCurrentDirectory=*/true));
    }

    try {

#if defined(PS_ENABLED)
      Gio::init();
      // the server object has to stay in scope
      // for having the server running
      boxed v = PS_Network_GioTcpServer_createServer()(1031)();
      PS_Server_server()
	(box<Sketch*>(static_cast<Sketch*>( &SolveSpace::SK)))
	//	 boxed( std::shared_ptr<Sketch*>
	//	(static_cast<Sketch*>( &SolveSpace::SK)
	//,[](Sketch*){}
	//	 ))) // empty deleter !
	(v)();
      auto s = PS_Solvespace_Resources_testSketch()();
      std::cout << "testSketch:" << unbox<std::string>( s) << std::endl;
      {
	boxed foo = std::shared_ptr<Foo>( new Foo);
      }
#endif

      testEntity("main 1");
      Platform::RunGui();
    } catch (std::exception& e) {

      std::cout << "error:" << e.what();
    }
      
    Platform::Close3DConnexion();
    SS.Clear();
    SK.Clear();
    Platform::ClearGui();

    return 0;
}

#if defined(WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow) {
    return main(0, NULL);
}
#endif
