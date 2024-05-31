#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << argv[0] << ": diskImageFile" << endl;
    return 1;
  }
  //super_t super;
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  super_t super;
  LocalFileSystem *lfs = new LocalFileSystem(disk);
  lfs->readSuperBlock(&super);

  cout << "Super" << endl;
  cout << "inode_region_addr " << super.inode_region_addr << endl;
  cout << "data_region_addr " << super.data_region_addr << endl;
  cout << endl;

  cout << "Inode bitmap" << endl;
  for(int i = 0; i < super.inode_bitmap_len; i++) {
    

   //cout << (unsigned int) super.bitmap[i] << " ";
  }
}
