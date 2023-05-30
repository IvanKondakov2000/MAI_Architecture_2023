#include "gc_server.h"

int main(int argc, char* argv[]) {
  HTTPGCServer app;
  return app.run(argc, argv);
}
