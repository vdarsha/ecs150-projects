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
  Disk disk = Disk(argv[1], UFS_BLOCK_SIZE);
  super_t super;
  LocalFileSystem lfs = LocalFileSystem(&disk);
  lfs.readSuperBlock(&super);
  unsigned char inodemap[UFS_BLOCK_SIZE * super.inode_bitmap_len];
  unsigned char datamap[UFS_BLOCK_SIZE * super.data_bitmap_len];

  lfs.readInodeBitmap(&super, inodemap);
  lfs.readDataBitmap(&super, datamap);
  

  cout << "Super" << endl;
  cout << "inode_region_addr " << super.inode_region_addr << endl;
  cout << "data_region_addr " << super.data_region_addr << endl;
  cout << endl;

  cout << "Inode bitmap\n";
  for(int i = 0; i < super.inode_bitmap_len*UFS_BLOCK_SIZE; i++) {
   cout << (unsigned int)  inodemap[i]<< " ";
  }
  cout << "\n\n";

  cout << "Data bitmap\n";
  for(int i = 0; i < super.data_bitmap_len*UFS_BLOCK_SIZE; i++) {
   cout << (unsigned int)  datamap[i]<< " ";
  }
  cout << '\n';
}
