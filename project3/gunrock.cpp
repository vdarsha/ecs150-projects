#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <fcntl.h>
#include <cstdlib>
#include <errno.h>



#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <deque>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unordered_map>
#include <queue>
#include <pthread.h>
#include <thread>
#include "dthread.h"






#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HttpService.h"
#include "HttpUtils.h"
#include "FileService.h"
#include "MySocket.h"
#include "MyServerSocket.h"
#include "dthread.h"

using namespace std;

int PORT = 8080;
int THREAD_POOL_SIZE = 50;
long unsigned int BUFFER_SIZE = 1;
string BASEDIR = "static";
string SCHEDALG = "FIFO";
string LOGFILE = "/dev/null";

vector<HttpService *> services;

//pthread_cond_t buff_space_free = PTHREAD_COND_INITIALIZER; //global condition variable
pthread_cond_t buff_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // global lock
queue<MySocket*> client_queue; //global queue




HttpService *find_service(HTTPRequest *request) {
   // find a service that is registered for this path prefix
  for (unsigned int idx = 0; idx < services.size(); idx++) {
    if (request->getPath().find(services[idx]->pathPrefix()) == 0) {
      return services[idx];
    }
  }

  return NULL;
}


void invoke_service_method(HttpService *service, HTTPRequest *request, HTTPResponse *response) {
  stringstream payload;

  // invoke the service if we found one
  if (service == NULL) {
    // not found status
    response->setStatus(404);
  } else if (request->isHead()) {
    service->head(request, response);
  } else if (request->isGet()) {
    service->get(request, response);
  } else {
    // The server doesn't know about this method
    response->setStatus(501);
  }
}

void *handle_request(void *arg) {
  
  while(true){
    MySocket *client;
    dthread_mutex_lock(&lock);
    while(client_queue.empty()){
      dthread_cond_wait(&buff_empty, &lock);
    }
    client = client_queue.front();
    client_queue.pop();
    dthread_mutex_unlock(&lock);
    //dthread_cond_signal(&buff_space_free);
    if (client == nullptr) 
    {
      continue;
    }
    
  
    HTTPRequest *request = new HTTPRequest(client, PORT);
    HTTPResponse *response = new HTTPResponse();
    stringstream payload;
    
    // read in the request
    bool readResult = false;
    try {
      payload << "client: " << (void *) client;
      sync_print("read_request_enter", payload.str());
      readResult = request->readRequest();
      sync_print("read_request_return", payload.str());
    } catch (...) {
      // swallow it
    }    
      
    if (!readResult) {
      // there was a problem reading in the request, bail
      delete response;
      delete request;
      sync_print("read_request_error", payload.str());
      continue;
    }
    
    HttpService *service = find_service(request);
    invoke_service_method(service, request, response);

    // send data back to the client and clean up
    payload.str(""); payload.clear();
    payload << " RESPONSE " << response->getStatus() << " client: " << (void *) client;
    sync_print("write_response", payload.str());
    cout << payload.str() << endl;
    client->write(response->response());
      
    delete response;
    delete request;

    payload.str(""); payload.clear();
    payload << " client: " << (void *) client;
    sync_print("close_connection", payload.str());
    client->close();
    delete client;
  }
  return NULL;
  
}



int main(int argc, char *argv[]) {

  signal(SIGPIPE, SIG_IGN);
  int option;
  


  while ((option = getopt(argc, argv, "d:p:t:b:s:l:")) != -1) {
    switch (option) {
    case 'd':
      BASEDIR = string(optarg);
      break;
    case 'p':
      PORT = atoi(optarg);
      break;
    case 't':
      THREAD_POOL_SIZE = atoi(optarg);
      break;
    case 'b':
      BUFFER_SIZE = atoi(optarg);
      break;
    case 's':
      SCHEDALG = string(optarg);
      break;
    case 'l':
      LOGFILE = string(optarg);
      break;
    default:
      cerr<< "usage: " << argv[0] << " [-p port] [-t threads] [-b buffers]" << endl;
      exit(1);
    }
  }

  set_log_file(LOGFILE);

  sync_print("init", "");
  MyServerSocket *server = new MyServerSocket(PORT);
  MySocket *client;



  // The order that you push services dictates the search order
  // for path prefix matching
  services.push_back(new FileService(BASEDIR));

  
  //queue<dthread_t> *thread_pool = new queue<dthread_t>();
  for(int i = 0; i < THREAD_POOL_SIZE; i++){
    pthread_t thread ;
    
    dthread_create(&thread, NULL, handle_request, NULL);
    //detatch(thread);
    //thread_pool->push(thread);
    
  }

  
  while(true) {
    sync_print("waiting_to_accept", "");
    client = server->accept();
    sync_print("client_accepted", ""); //you can accept client and then wait until there is space in the queue to add it
    if(client == nullptr){
      continue;
    }
    dthread_mutex_lock(&lock);
    while (client_queue.size() >= BUFFER_SIZE){
      // wait for a thread to finish
      dthread_cond_wait(&buff_empty, &lock);

    }

    client_queue.push(client);
    dthread_cond_signal(&buff_empty);
    dthread_mutex_unlock(&lock);
    

    //handle_request(client);
  }
  return 0;
}
