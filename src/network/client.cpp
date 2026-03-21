#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h> // for usleep()

using namespace std;

//GLOBAL VARIABLES
queue<string> message_queue;
mutex queue_mutex;

//sets up the socket and connects to the server
//returns the connected socket file descriptor, or -1 on failure
int connect_to_server(const char *host, const char *port);
int send_all(int sockfd, const char *buf, int *len); //send
void listen_to_server(int sockfd); // function to run in the background thread


int main()
{
  int sockfd;

  //connect to localhost on the same port the server is listening on
  sockfd = connect_to_server("127.0.0.1", "8080");

  if (sockfd == -1) // we can call the function above inside the if statement
  {
    cerr << "Failed to connect to the server." << endl;
    return -1;
  }
  cout << "Client: connected to server!" << endl;

  //launch the backgroud thread and detach it
  thread listener(listen_to_server, sockfd);
  listener.detach();

  //main loop
  while(true)
  {
    string incoming_msg = "";
    bool has_message = false;

    //start critical section
    //When pulling data out of the queue, the trick is to keep the mutex locked
    //for the shortest amount of time possible.
    //You lock it, grab the data, delete it from the queue, and immediately
    //unlock it. Then, you can take your time printing or processing the string.
    queue_mutex.lock();
    if(!message_queue.empty())
    {
      //copy the message to our local variable
      incoming_msg = message_queue.front();

      //remove it from the shared queue
      message_queue.pop();

      has_message = true;
    }

    //unlock as soon as we are done touching the shared queue
    queue_mutex.unlock();
    //end critical section

    //process the data outside the lock
    if(has_message)
    {
      cout << "Main Thread Processed: " << incoming_msg << endl;

      //add game logic, ui updates, or commands here!
      if(incoming_msg == "SERVER_SHUTDOWN")
      {
        break;
      }
    }

    // take a tiny nap (10 milliseconds)
    // this prevents this empty while loop from spinning millions 
    // of times a second and maxing out your cpu
    usleep(10000);
  }
  close(sockfd);
  return 0;
}







//sets up the socket and connects to the server
//returns the connected socket file descriptor, or -1 on failure
int connect_to_server(const char *hostname, const char *port)
{
  struct addrinfo hints, *res;
  int sockfd;

  //configure socket parameters
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  //resolve the server address and port
  if(getaddrinfo(hostname, port, &hints, &res) != 0)
  {
    cerr << "1" << endl;
    perror("getaddrinfo");
    return -1;
  }

  //create the socket descriptor
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(sockfd == -1)
  {
    cerr << "2" << endl;
    perror("socket");
    freeaddrinfo(res);
    return -1;
  }

  //connect to the server
  //unlike the server, we don't bind or listen. We just connect to the
  //address returned by getaddrinfo
  if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
  {
    cerr << "3" << endl;
    perror("connect");
    close(sockfd);
    freeaddrinfo(res);
    return -1;
  }
  
  //we can free this as soon as we are done connecting
  freeaddrinfo(res);

  return sockfd;
}


//send the message through the socket
//handles partial sends
//dosenst need to convert to network byte order because
//we are sending chars (already 1 byte)
// - sockfd is the sock descriptor
// - buf is the string to send (as a char *)
// - len in a pointer to an int containing the number of bytes
//in the buffer
int send_all(int sockfd, const char *buf, int *len)
{
  int total = 0;         // how many bytes we've sent
  int bytes_left = *len; // how many bytes we have left to send
  int n;

  while(total < *len)
  {
    // because buf is a pointer, we do pointer arithmetic
    // and send from where ever the buffer is + how ever much
    // data was sent so far
    // bytes_left tells how many bytes of data to send
    //
    n = send(sockfd, buf + total, bytes_left, 0);
    if(n == -1)
    {
      break;
    }
    total += n;
    bytes_left -= n;
  }

  *len = total; // return the number of bytes actually sent
  
  return n==-1 ? -1 : 0; // return -1 on failure, 0 on success
}

// function to run in the background thread
void listen_to_server(int sockfd)
{
  char buf[1024];
  while(true)
  {
    int bytes_received = recv(sockfd, buf, sizeof(buf) - 1, 0);

    if(bytes_received <= 0)
    {
      cout << "\nDisconnected from server" << endl;
      break;
    }

    //safely terminate the c-string
    buf[bytes_received] = '\0';

    //convert it to a c++ string so its easier to store
    string server_message(buf);

    //hand off to main thread
    queue_mutex.lock();

    //drop the message into the queue
    message_queue.push(server_message);

    //unlock the mailbox so the main thread can read it
    queue_mutex.unlock();
  }
}



