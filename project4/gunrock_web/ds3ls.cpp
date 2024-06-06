#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <vector>


#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

void getDir(LocalFileSystem &lfs, int inodeNumber, const string &path) {
  inode_t inode;
  if(lfs.stat(inodeNumber, &inode) != 0) {
    cout << "Invalid inode number " << inodeNumber << endl;
    return;
  }
  if(inode.type != UFS_DIRECTORY) {
    cout << "Not a directory" << endl;
    return;
  }
  vector<dir_ent_t> entries;
  char buffer[inode.size];
  if(lfs.read(inodeNumber, &buffer, inode.size) != inode.size) {
    cout << "Error reading directory" << endl;
    return;
  }
  for(int i = 0; i < inode.size/int(sizeof(dir_ent_t)); i++) {
    dir_ent_t entry;
    memcpy(&entry, buffer + i*sizeof(dir_ent_t), sizeof(dir_ent_t));
    entries.push_back(entry);
  }

  sort(entries.begin(), entries.end(), [](const dir_ent_t &a, const dir_ent_t &b) {
    return strcmp(a.name, b.name) < 0;
  });
  cout << "Directory " << path << "/" << endl;
  for (const auto &entry : entries) {
    cout << entry.inum << "\t" << entry.name << endl;
  }
  cout << endl;

  for(const auto &entry : entries) {
    if(strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0){
      continue;
    }
    if(entry.inum < 0) {
      cout << "Invalid inode number " << entry.inum << endl;
      continue;
    }
    if(lfs.stat(entry.inum, &inode) == 0 && inode.type == UFS_DIRECTORY) {
      getDir(lfs, entry.inum, path + "/" + entry.name);
      
      
    }
    
  }

  
}
int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << argv[0] << ": diskImageFile" << endl;
    return 1;
  }
  Disk disk = Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem lfs = LocalFileSystem(&disk);
  getDir(lfs, 0, "");

}
