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
  super_t super;
  readSuperBlock(&super);
  inode_t inode;
  if(parentInodeNumber < 0 || parentInodeNumber > super.num_inodes - 1) {
    return -EINVALIDINODE;
  }
  if(stat(parentInodeNumber, &inode) != 0) {
    return -EINVALIDINODE;
  }
  if(inode.type != UFS_DIRECTORY) {
    return -EINVALIDINODE;
  }
  char buffer[inode.size];
  vector<dir_ent_t> entries;
  read(parentInodeNumber, &buffer, inode.size);
  for(int i = 0; i < inode.size/int(sizeof(dir_ent_t)); i++) {
    dir_ent_t entry;

    memcpy(&entry, buffer + i*sizeof(dir_ent_t), sizeof(dir_ent_t));
    entries.push_back(entry);
  }
  
  for(const auto &entry : entries) {
    if(strcmp(entry.name, name.c_str()) == 0) {
      return entry.inum;
    }
  }

  return -ENOTFOUND;
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
  if(size <= UFS_BLOCK_SIZE){ //for small files
    int blocks = inode.direct[0];
    char block[UFS_BLOCK_SIZE];
    disk->readBlock(blocks, block);
    memcpy(buffer, block, size);
    return size;
  }

  int blocks = size/UFS_BLOCK_SIZE;
  if(size % UFS_BLOCK_SIZE != 0) {
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
  if(size % UFS_BLOCK_SIZE != 0) {
    sizer = size % UFS_BLOCK_SIZE;
  }

  memcpy(buff2 + ((blocks-1) * UFS_BLOCK_SIZE), &buff, sizer);
  memcpy(buffer, &buff2, size);
  


  return size;
}

int LocalFileSystem::create(int parentInodeNumber, int type, string name) {
  super_t super;
  readSuperBlock(&super);
  if(name.empty() || name.size() >= DIR_ENT_NAME_SIZE){
    return -EINVALIDNAME;
  }

  if(type != UFS_REGULAR_FILE && type != UFS_DIRECTORY){
    return -EINVALIDTYPE;
  }

  inode_t parent_inode;
  if(this->stat(parentInodeNumber, &parent_inode) != 0 || parent_inode.type != UFS_DIRECTORY){
    return -EINVALIDINODE;
  }
  if(parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes){
    return -EINVALIDINODE;
  }

  int num_entries = parent_inode.size / sizeof(dir_ent_t);
  dir_ent_t entries[num_entries];
  if(this->read(parentInodeNumber, entries, parent_inode.size) != parent_inode.size){
    return -EINVALIDINODE;
  }
  for(int i = 0; i < num_entries; i++){
    dir_ent_t entry = entries[i];
    if(entry.name == name)
    {
      inode_t inode;
      stat(entry.inum, &inode);
      if(inode.type == type)
      {
        return entry.inum;
      }
      else
      {
        return -EINVALIDTYPE;
      }
    }
  }

  if((parent_inode.size % UFS_BLOCK_SIZE) == 0){
    if(!diskHasSpace(&super, 1, 0, 2))
    {
      return -ENOTENOUGHSPACE;
    }
  }
  else
  {
    if(!diskHasSpace(&super, 1, 0, 1))
    {
      return -ENOTENOUGHSPACE;
    }
  }

  int i_bit_size = super.inode_bitmap_len * UFS_BLOCK_SIZE;
  vector<unsigned char> inode_bitmap(i_bit_size);
  readInodeBitmap(&super, inode_bitmap.data());

  int inum_new = -1;
  for(int i = 0; i < super.num_inodes; i++){
    if(!(inode_bitmap[i / 8] & (1 << (i % 8))))
    {
      inode_bitmap[i / 8] |= (1 << i % 8);
      inum_new = i;
      break;
    }
    if(inum_new != -1)
    {
      break;
    }
  }

  if(inum_new == -1){
    return -ENOTENOUGHSPACE;
  }

  inode_t inode_new;
  memset(&inode_new, 0, sizeof(inode_t));
  inode_new.type = type;
  inode_new.size = 0;

  int block_num = -1;
  if(type == UFS_DIRECTORY){
    vector<unsigned char> dataBitmap(super.data_bitmap_len * UFS_BLOCK_SIZE);
    readDataBitmap(&super, dataBitmap.data());

    for(int i = 0; i < super.num_data; ++i)
    {
      if(!(dataBitmap[i / 8] & (1 << (i % 8))))
      {
        block_num = i;
        dataBitmap[i / 8] |= (1 << (i % 8));
        break;
      }
    }

    if(block_num == -1)
    {
      inode_bitmap[inum_new / 8] &= ~(1 << (inum_new % 8));
      writeInodeBitmap(&super, inode_bitmap.data());
      return -ENOTENOUGHSPACE;
    }

    inode_new.direct[0] = super.data_region_addr + block_num;
    dir_ent_t entries1[2];
    memset(entries1, 0, sizeof(entries1));
    strncpy(entries1[0].name, ".", DIR_ENT_NAME_SIZE - 1);
    
    entries1[0].name[DIR_ENT_NAME_SIZE - 1] = '\0';
    entries1[0].inum = inum_new;

    strncpy(entries1[1].name, "..", DIR_ENT_NAME_SIZE - 1);
    entries1[1].name[DIR_ENT_NAME_SIZE - 1] = '\0';
    entries1[1].inum = parentInodeNumber;

    disk->writeBlock(inode_new.direct[0], entries1);
    inode_new.size = sizeof(entries1);
    writeDataBitmap(&super, dataBitmap.data());
  }

  vector<inode_t> inode_table(super.num_inodes);
  readInodeRegion(&super, inode_table.data());
  inode_table[inum_new] = inode_new;
  writeInodeRegion(&super, inode_table.data());

  dir_ent_t new_entry;
  memset(&new_entry, 0, sizeof(new_entry));
  name += "\0";
  strncpy(new_entry.name, name.c_str(), DIR_ENT_NAME_SIZE);
  new_entry.inum = inum_new;
  vector<char> buffer(parent_inode.size + UFS_BLOCK_SIZE - 1, 0);

  if(this->read(parentInodeNumber, buffer.data(), parent_inode.size) != parent_inode.size){
    inode_bitmap[inum_new / 8] &= ~(1 << (inum_new % 8));
    writeInodeBitmap(&super, inode_bitmap.data());

    return -EINVALIDINODE;
  }

  if(parent_inode.size == UFS_BLOCK_SIZE){ 
    vector<unsigned char> data_bitmap(super.data_bitmap_len * UFS_BLOCK_SIZE);
    readDataBitmap(&super, data_bitmap.data());

    int parentNewBlockIndex = -1;
    for(int i = 0; i < super.num_data; ++i)
    {
      if(!(data_bitmap[i / 8] & (1 << (i % 8))))
      {
        parentNewBlockIndex = i;
        data_bitmap[i / 8] |= (1 << (i % 8));
        break;
      }
    }

    if(parentNewBlockIndex == -1)
    {
      inode_bitmap[inum_new / 8] &= ~(1 << (inum_new % 8));
      writeInodeBitmap(&super, inode_bitmap.data());
      return -ENOTENOUGHSPACE;
    }

    for(int i = 0; i < DIRECT_PTRS; ++i)
    {
      if(parent_inode.direct[i] == 0)
      {
        parent_inode.direct[i] = super.data_region_addr + parentNewBlockIndex;
        break;
      }
    }

    writeDataBitmap(&super, data_bitmap.data());
  }

  memcpy(buffer.data() + parent_inode.size, &new_entry, sizeof(dir_ent_t));
  parent_inode.size += sizeof(dir_ent_t);

  inode_table[parentInodeNumber].type = UFS_REGULAR_FILE;
  writeInodeRegion(&super, inode_table.data());

  if(this->write(parentInodeNumber, buffer.data(), parent_inode.size) != parent_inode.size){
    int ibx = inum_new / 8;
    int ibofs = inum_new % 8;
    inode_bitmap[ibx] &= ~(1 << ibofs);
    writeInodeBitmap(&super, inode_bitmap.data());
    if(type == UFS_DIRECTORY)
    {
      vector<unsigned char> dataBitmap(super.data_bitmap_len * UFS_BLOCK_SIZE);
      readDataBitmap(&super, dataBitmap.data());

      int block_idx = (inode_new.direct[0] - super.data_region_addr) / UFS_BLOCK_SIZE;
      int block_ofs = (inode_new.direct[0] - super.data_region_addr) % UFS_BLOCK_SIZE;

      dataBitmap[block_idx] &= ~(1 << block_ofs);
      writeDataBitmap(&super, dataBitmap.data());
    }

    inode_table[parentInodeNumber].type = UFS_DIRECTORY;
    writeInodeRegion(&super, inode_table.data());

    return -ENOTENOUGHSPACE;
  }

  writeInodeBitmap(&super, inode_bitmap.data());

  readInodeRegion(&super, inode_table.data());
  inode_table[parentInodeNumber] = parent_inode;
  writeInodeRegion(&super, inode_table.data());

  return inum_new;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {

  super_t super;
  readSuperBlock(&super);
  inode_t inode;
  if(inodeNumber < 0 || inodeNumber >= super.num_inodes) {
      return -EINVALIDINODE;
  }
  if(stat(inodeNumber, &inode) != 0) {
      return -EINVALIDINODE;
  }
  if(inode.type != UFS_REGULAR_FILE) {
      return -EINVALIDINODE;
  }
  if(size > MAX_FILE_SIZE) {
      return -EINVALIDSIZE;
  }
  int blocks = (size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
  if(blocks > 12) {
      return -ENOTENOUGHSPACE;
  }
  const char* buff = static_cast<const char*>(buffer);
  for(int i = 0; i < blocks; ++i) {
      char blockBuffer[UFS_BLOCK_SIZE] = {0};
      int copySize = min(size - (i * UFS_BLOCK_SIZE), UFS_BLOCK_SIZE);
      memcpy(blockBuffer, buff + (i * UFS_BLOCK_SIZE), copySize);
      disk->writeBlock(inode.direct[i], blockBuffer);
  }


  inode.size = size;
  int blockNumber = (inodeNumber * sizeof(inode_t)) / UFS_BLOCK_SIZE + super.inode_region_addr;
  int offset = (inodeNumber * sizeof(inode_t)) % UFS_BLOCK_SIZE;
  char inodeBuffer[UFS_BLOCK_SIZE];
  disk->readBlock(blockNumber, inodeBuffer);
  memcpy(inodeBuffer + offset, &inode, sizeof(inode_t));
  disk->writeBlock(blockNumber, inodeBuffer);

  return size;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name) {
  super_t super;
  inode_t parent_inode;
  readSuperBlock(&super);
  if(stat(parentInodeNumber, &parent_inode) != 0 || parent_inode.type != UFS_DIRECTORY) {
    return -EINVALIDINODE;
  }
  if(name.empty() || name == "." || name == "..") {
    return -EUNLINKNOTALLOWED;
  }
  if(name.length() > DIR_ENT_NAME_SIZE) {
    return -EINVALIDNAME;
  }

  unsigned char bitmap[super.inode_bitmap_len * UFS_BLOCK_SIZE];
  readInodeBitmap(&super, bitmap);

  if((bitmap[parentInodeNumber/8] & (1 << (parentInodeNumber % 8))) == 0) {
    return -EINVALIDINODE;
  }

  inode_t del_inode;
  int del_inum = lookup(parentInodeNumber, name);
  if(del_inum == -ENOTFOUND) {
    return 0;
  }

  stat(del_inum, &del_inode);
  if(del_inode.type == UFS_DIRECTORY) {
    if(del_inode.size > static_cast<int>(2 * sizeof(dir_ent_t))) {
      return -ENOTEMPTY;
    }
  }

  bitmap[del_inum/8] &= ~(1 << (del_inum % 8));
  writeInodeBitmap(&super, bitmap);
  unsigned char dataBitmap[super.data_bitmap_len * UFS_BLOCK_SIZE];
  readDataBitmap(&super, dataBitmap);

  int num_blocks = del_inode.size/UFS_BLOCK_SIZE;
  if(del_inode.size % UFS_BLOCK_SIZE != 0){
    num_blocks++;
  }
  for(int i = 0; i < num_blocks; i++){
    dataBitmap[del_inode.direct[i]/8] &= ~(1 << (del_inode.direct[i] % 8));
  }
  writeDataBitmap(&super, dataBitmap);
  vector<dir_ent_t> entry(parent_inode.size/sizeof(dir_ent_t));
  read(parentInodeNumber, entry.data(), parent_inode.size);
  for(auto num = entry.begin(); num != entry.end();){
    if(num->name == name)
    {
      num = entry.erase(num);
    }
    else
    {
      ++num;
    }
  }

  unsigned char *buf = reinterpret_cast<unsigned char *>(entry.data());
  for(int i = 0, size = parent_inode.size; size > 0; i++){
    disk->writeBlock(parent_inode.direct[i], buf);
    buf += UFS_BLOCK_SIZE;
    size -= UFS_BLOCK_SIZE;
  }
  parent_inode.size -= sizeof(dir_ent_t);
  inode_t nodes[super.num_inodes];
  readInodeRegion(&super, nodes);
  nodes[parentInodeNumber] = parent_inode;
  writeInodeRegion(&super, nodes);

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
  for(int i = 0; i < super->inode_bitmap_len; i++) {
    disk->writeBlock(super->inode_bitmap_addr + i, inodeBitmap + (i * UFS_BLOCK_SIZE));
  }
}
void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap){
  for(int i = 0; i < super->data_bitmap_len; i++) {
    disk->writeBlock(super->data_bitmap_addr + i, dataBitmap + (i * UFS_BLOCK_SIZE));
  }
}
void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes){
  for(int i = 0; i < super->inode_region_len; i++) {
    disk->writeBlock(super->inode_region_addr + i, inodes + (i * UFS_BLOCK_SIZE));
  }
}

bool LocalFileSystem::diskHasSpace(super_t *super, int numInodesNeeded, int numDataBytesNeeded, int numDatablocks){
  vector<unsigned char> dataBitmap(super->data_bitmap_len * UFS_BLOCK_SIZE, 0);
  readDataBitmap(super, dataBitmap.data());
  numDatablocks += numDataBytesNeeded / UFS_BLOCK_SIZE + (numDataBytesNeeded % UFS_BLOCK_SIZE != 0);
  int dcounter = 0;
  for(int i = 0; i < super->num_data; i++){
    for(int j = 0; j < 8; j++){
      if((dataBitmap[i] & (1 << j)) == 0){
        dcounter++;
      }
    }
  }
  vector<unsigned char> inodeBitmap(super->inode_bitmap_len * UFS_BLOCK_SIZE, 0);
  readInodeBitmap(super, inodeBitmap.data());
  int icounter= 0;
  for(int i = 0; i < super->num_inodes; i++){
    for(int j = 0; j < 8; j++){
      if((inodeBitmap[i] & (1 << j)) == 0){
        icounter++;
      }
    }
  }
  return ((super->num_data - dcounter) >= numDatablocks && (super->num_inodes - icounter) >= numInodesNeeded);

  
}