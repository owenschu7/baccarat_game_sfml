#include "ServerApplication.h"

#include "../../core/Debug.h"

int main() 
{
  DEBUG_PRINT << "server.cpp: main(): Booting up Server...\n";

  // Initialize the transport layer on port 8080
  ServerApplication app("8080"); 
  
  // Start the infinite poll() loop
  app.run(); 

  return 0;
}
