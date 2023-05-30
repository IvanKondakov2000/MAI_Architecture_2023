#include "p2p_server.h"

int main(int argc, char* argv[]) {
  HTTPP2PServer app;
  return app.run(argc, argv);
}
