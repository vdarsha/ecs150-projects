#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <cstring>

#include "LocalFileSystem.h"
#include "ufs.h"

using namespace std;


LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}

void LocalFileSystem::readSuperBlock(super_t *super) {
  char buffer[UFS_BLOCK_SIZE];
  disk->readBlock(0, &buffer);
  memcpy(super, &buffer, sizeof(super_t));
}

int LocalFileSystem::lookup(int parentInodeNumber, string name) {


  return 0;
}

int LocalFileSystem::stat(int inodeNumber, inode_t *inode) {
  super_t super;
  
  readSuperBlock(&super);
  if(inodeNumber < 0 || inodeNumber > super.num_inodes - 1) {
    return -EINVALIDINODE;
  }

  inode_t inodes[super.inode_region_len*UFS_BLOCK_SIZE];
  for(int i = 0; i < super.inode_region_len; i++) {
    disk->readBlock(super.inode_region_addr + i, inodes + (i * UFS_BLOCK_SIZE));

  }
  
  memcpy(inode, &inodes[inodeNumber], sizeof(inode_t));

  return 0;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size) {
  super_t super;
  readSuperBlock(&super);
  inode_t inode;
  if(inodeNumber < 0 || inodeNumber > super.num_inodes - 1) {
    return -EINVALIDINODE;
  }
  if(stat(inodeNumber, &inode) != 0) {
    return -EINVALIDINODE;
  }
  
  if(size < 0 || size > MAX_FILE_SIZE) {
    return -EINVALIDSIZE;
  }
  if(size > inode.size) {
    size = inode.size;
  }
  //cout << "File blocks\n";
  // if(inode.type == UFS_DIRECTORY) {
  //   char buff[inode.size];
  //   for(int i = 0; i < inode.size/UFS_BLOCK_SIZE - 1; i++) {
  //     dir_ent_t dir;
  //     disk->readBlock(inode.direct[i], &dir);
  //     //cout << inode.direct[i] << '\n';
  //     memcpy(buff + (i * DIR_ENT_NAME_SIZE), &dir.name, dir.name[DIR_ENT_NAME_SIZE - 1]);
  //   }
  //   memcpy(buffer, &buff, size);

  // }
  // else if(inode.type == UFS_REGULAR_FILE) {
  //   char buff[MAX_FILE_SIZE];
    
  //   for(int i = 0; i <= (inode.size/UFS_BLOCK_SIZE); i++) {
  //     disk->readBlock(inode.direct[i], buff + (i * UFS_BLOCK_SIZE));
  //     //cout << inode.direct[i] << '\n';
  //   }
  //   memcpy(buffer, &buff, size);

  // }
  if (size <= UFS_BLOCK_SIZE){ //for small files
    int blocks = inode.direct[0];
    char block[UFS_BLOCK_SIZE];
    disk->readBlock(blocks, block);
    memcpy(buffer, block, size);
    return size;
  }

  int blocks = size/UFS_BLOCK_SIZE;
  if (size % UFS_BLOCK_SIZE != 0) {
    blocks++;
  }
  char buff2[size];
  for(int i = 0; i < blocks - 1; i++) {
    char buff[UFS_BLOCK_SIZE];
    disk->readBlock(inode.direct[i], buff);
    //cout << inode.direct[i] << '\n';
    memcpy(buff2 + (i * UFS_BLOCK_SIZE), &buff, UFS_BLOCK_SIZE);
  }
  char buff[UFS_BLOCK_SIZE];
  disk->readBlock(inode.direct[blocks - 1], buff);
  int sizer = size;
  if (size % UFS_BLOCK_SIZE != 0) {
    sizer = size % UFS_BLOCK_SIZE;
  }

  memcpy(buff2 + ((blocks-1) * UFS_BLOCK_SIZE), &buff, sizer);
  memcpy(buffer, &buff2, size);
  


  return size;
}

int LocalFileSystem::create(int parentInodeNumber, int type, string name) {
  return 0;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
  return 0;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name) {
  return 0;
}
void LocalFileSystem::readInodeBitmap(super_t *super, unsigned char *inodeBitmap){
  unsigned char buffer[UFS_BLOCK_SIZE];
  int offset = 0;
  for(int i = 0; i < super->inode_bitmap_len; i++) {
    disk->readBlock(super->inode_bitmap_addr + i, &buffer);
    memcpy(inodeBitmap + offset, buffer, UFS_BLOCK_SIZE);
    offset += UFS_BLOCK_SIZE;
  }
}

void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap){
  unsigned char buffer[UFS_BLOCK_SIZE];
  int offset = 0;
  for(int i = 0; i < super->data_bitmap_len; i++) {
    disk->readBlock(super->data_bitmap_addr + i, &buffer);
    memcpy(dataBitmap + offset, buffer, UFS_BLOCK_SIZE);
    offset += UFS_BLOCK_SIZE;
  }
}

void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes){
  unsigned char buffer[UFS_BLOCK_SIZE];
  int offset = 0;
  for(int i = 0; i < super->inode_region_len; i++) {
    disk->readBlock(super->inode_region_addr + i, &buffer);
    memcpy(inodes + offset, buffer, UFS_BLOCK_SIZE);
    offset += UFS_BLOCK_SIZE;
  }
}
void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap){

}
void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap){

}
void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes){

}
