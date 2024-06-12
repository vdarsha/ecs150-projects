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
  
  unsigned char file[inode.size];
  int x = atoi(argv[2]);
  
  if(lfs.stat(x, &inode) != 0) {
    cout << "Invalid inode number " << x << endl;
    return -1;
  }
  cout << "File blocks\n";
  int datasize = inode.size/UFS_BLOCK_SIZE;
  if(inode.size % UFS_BLOCK_SIZE != 0){
    datasize++;
  }
  for(int i = 0; i < datasize; i++){
    cout << inode.direct[i] << '\n';
  }
  cout << "\n";
  lfs.read(x, file, inode.size);

  cout << "File data\n";
  cout << file;


  // write testing

  // int bytesWritten = lfs.write(x, file, inode.size + 1);

  // if (bytesWritten >= 0) {
  //     // Write successful
  //     cout << "Bytes written: " << bytesWritten << endl;
  // } else {
  //     // Write failed, handle the error
  //     switch (bytesWritten) {
  //         case -EINVALIDINODE:
  //             cout << "Error: Invalid inode number." << endl;
  //             break;
  //         case -EINVALIDTYPE:
  //             cout << "Error: Not a regular file." << endl;
  //             break;
  //         case -EINVALIDSIZE:
  //             cout << "Error: Invalid size." << endl;
  //             break;
  //         case -ENOTENOUGHSPACE:
  //             cout << "Error: Not enough space." << endl;
  //             break;
  //         // Handle other possible error codes...
  //         default:
  //             cout << "Error: Unknown error occurred." << endl;
  //     }
  // }

  return 0;


  


  
}
