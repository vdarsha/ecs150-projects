#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cout << argv[0] << ": diskImageFile inodeNumber" << endl;
    return 1;
  }
  Disk disk = Disk(argv[1], UFS_BLOCK_SIZE);
  super_t super;
  LocalFileSystem lfs = LocalFileSystem(&disk);
  lfs.readSuperBlock(&super);
  inode_t inode;
  
  char file[MAX_FILE_SIZE];
  int x = atoi(argv[2]);
  
  lfs.stat(x, &inode);
  lfs.read(x, &file, inode.size);
  cout << "File data\n";
  cout << file << '\n';
  return 0;


  
}