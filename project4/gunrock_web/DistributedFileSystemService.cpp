#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <algorithm>

#include "DistributedFileSystemService.h"
#include "ClientError.h"
#include "ufs.h"
#include "WwwFormEncodedDict.h"

using namespace std;

DistributedFileSystemService::DistributedFileSystemService(string diskFile) : HttpService("/ds3/") {
  this->fileSystem = new LocalFileSystem(new Disk(diskFile, UFS_BLOCK_SIZE));
}  

void DistributedFileSystemService::get(HTTPRequest *request, HTTPResponse *response) {
  response->setBody("");
  // fileSystem->disk->beginTransaction();
  // vector<string> path = request->getPathComponents(); //ignore first entry
  // int parent = 0;
  // for(unsigned long int i = 1; i < path.size(); i++){
  //   parent = fileSystem->lookup(parent, path[i]);

  // }
  // inode_t inode;
  // fileSystem->stat(parent, &inode);
  // if(inode.type == UFS_REGULAR_FILE){
  //   char buffer[inode.size];
  //   int size = fileSystem->read(parent, buffer, MAX_FILE_SIZE);
  //   response->setBody(string(buffer, size));
  // }
  // else{
  //   vector<dir_ent_t> entries;
  //   for(int)
  //   fileSystem->read(parent, entries[i]);
  //   stringstream ss;
  //   for(unsigned long int i = 0; i < entries.size(); i++){
  //     ss << entries[i].name << "\n";
  //   }
  //   response->setBody(ss.str());
  // }


}

void DistributedFileSystemService::put(HTTPRequest *request, HTTPResponse *response) {
  response->setBody("");
  fileSystem->disk->beginTransaction();

  vector<string> path = request->getPathComponents(); //ignore first entry
  int parent = 0;
  for(unsigned long int i = 1; i < path.size() - 1; i++){
    if((parent = fileSystem->lookup(parent, path[i])) < 0){
      break;
    }
    if((parent = fileSystem->create(parent, UFS_DIRECTORY, path[i])) < 0){
      fileSystem->disk->rollback();
      return;
    }
  }
  if((parent = fileSystem->create(parent, UFS_REGULAR_FILE, path[path.size() - 1])) < 0){
    fileSystem->disk->rollback();
    return;
  }
  int bsize = request->getBody().size();
  if(bsize > 0){
    bsize ++;
  }

  if(fileSystem->write(parent, request->getBody().c_str(), (bsize + 1)) < 0){
    fileSystem->disk->rollback();
    return;
  
  }
  
  fileSystem->disk->commit();
}

void DistributedFileSystemService::del(HTTPRequest *request, HTTPResponse *response) {
  response->setBody("");
}
