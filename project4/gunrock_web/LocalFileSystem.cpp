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

  inode_t inodes[super.inode_region_len*UFS_BLOCK_SIZE];
  for(int i = 0; i < super.inode_region_len; i++) {
    disk->readBlock(super.inode_region_addr + i, inodes + (i * UFS_BLOCK_SIZE));

  }
  if(inodeNumber < 0 || inodeNumber >= super.num_inodes) {
    return EINVALIDINODE;
  }
  memcpy(inode, &inodes[inodeNumber], sizeof(inode_t));

  return 0;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size) {
  super_t super;
  readSuperBlock(&super);
  inode_t inode;
  stat(inodeNumber, &inode);
  if(inodeNumber < 0 || inodeNumber >= super.num_inodes) {
    return EINVALIDINODE;
  }
  if(size > inode.size || size < 0) {
    return EINVALIDSIZE;
  }
  cout << "File blocks\n";
  if(inode.type == UFS_DIRECTORY) {
    dir_ent_t dir;
    for(int i = 0; i < sizeof(inode.direct) - 1; i++) {
      disk->readBlock(super.data_region_addr + inode.direct[i], &dir);
      cout << inode.direct[i] << '\n';
      
    }
    memcpy(buffer, &dir, sizeof(dir_ent_t));

  }
  else if(inode.type == UFS_REGULAR_FILE) {
    char buff[MAX_FILE_SIZE];
    for(int i = 0; i < sizeof(inode.direct) - 1; i++) {
      disk->readBlock(super.data_region_addr + inode.direct[i], buff + (i * UFS_BLOCK_SIZE));
      cout << inode.direct[i] << '\n';
    }
    memcpy(buffer, &buff, size);

  }

  return 0;
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
void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap){

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
void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap){

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
void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes){

}
