#include <stdexcept>
#include <sys/stat.h>

#include "glog/logging.h"
#include "page/bitmap_page.h"
#include "storage/disk_manager.h"

DiskManager::DiskManager(const std::string &db_file) : file_name_(db_file) {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
  db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
  // directory or file does not exist
  if (!db_io_.is_open()) {
    db_io_.clear();
    // create a new file
    db_io_.open(db_file, std::ios::binary | std::ios::trunc | std::ios::out);
    db_io_.close();
    // reopen with original mode
    db_io_.open(db_file, std::ios::binary | std::ios::in | std::ios::out);
    if (!db_io_.is_open()) {
      throw std::exception();
    }
  }
  ReadPhysicalPage(META_PAGE_ID, meta_data_);
}

void DiskManager::Close() {
  std::scoped_lock<std::recursive_mutex> lock(db_io_latch_);
  if (!closed) {
    db_io_.close();
    closed = true;
  }
}

void DiskManager::ReadPage(page_id_t logical_page_id, char *page_data) {
  ASSERT(logical_page_id >= 0, "Invalid page id.");
  ReadPhysicalPage(MapPageId(logical_page_id), page_data);
}

void DiskManager::WritePage(page_id_t logical_page_id, const char *page_data) {
  ASSERT(logical_page_id >= 0, "Invalid page id.");
  WritePhysicalPage(MapPageId(logical_page_id), page_data);
}

page_id_t DiskManager::AllocatePage() {
  // 由于是顺序分配页，那么我们就可以根据allocatedPages找到要分配的extent_id
  DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(GetMetaData());
  uint32_t extent_id = meta_page->GetAllocatedPages() / BITMAP_SIZE;
  // 根据extent_id找到对应的bitmap_page页
  uint32_t bitmap_page = extent_id * BITMAP_SIZE + 1;
  // 从磁盘中读出bitmap_page
  char page_data[PAGE_SIZE];
  ReadPhysicalPage(bitmap_page, page_data);
  BitmapPage<PAGE_SIZE> *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(page_data);
  // 分配页并得到page_id
  uint32_t page_offset;
  bitmap->AllocatePage(page_offset);
  uint32_t logical_page_id = page_offset + extent_id * BITMAP_SIZE;
  meta_page->num_allocated_pages_++;
//  if((meta_page->GetAllocatedPages() - 1) % BITMAP_SIZE == 0){
//    int num = meta_page->GetAllocatedPages() / BITMAP_SIZE + 1;
//    meta_page->num_extents_++;
//    //meta_page->extent_used_page_ = (uint32_t *)malloc(sizeof(uint32_t) * num);
//    meta_page->extent_used_page_[num - 1] = extent_id * BITMAP_SIZE + 1;
//  }
  // 更新meta_page中的信息
  meta_page->num_extents_ = extent_id + 1;
  meta_page->extent_used_page_[extent_id] = bitmap->GetAllocatedPage();
  // 非常重要，记得写回到磁盘中！！！
  WritePhysicalPage(bitmap_page, page_data);
  return logical_page_id;
}

void DiskManager::DeAllocatePage(page_id_t logical_page_id) {
  // 清空页内容
//  char* clear = nullptr;
//  WritePage(logical_page_id, clear);
  // 找到对应的bitmap_page
  uint32_t extent_id = logical_page_id / BITMAP_SIZE;
  uint32_t offset = logical_page_id % BITMAP_SIZE;
  DiskFileMetaPage *meta_page = reinterpret_cast<DiskFileMetaPage *>(GetMetaData());
  uint32_t bitmap_page = extent_id * BITMAP_SIZE + 1;
  // 读磁盘得到对应bitmap_page的数据
  char page_data[PAGE_SIZE];
  ReadPhysicalPage(bitmap_page, page_data);
  BitmapPage<PAGE_SIZE> *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(page_data);
  bitmap->DeAllocatePage(offset);
  // 更新meta_page中的内容
  meta_page->num_allocated_pages_--;
  meta_page->num_extents_ = meta_page->GetAllocatedPages() / BITMAP_SIZE + 1;
  meta_page->extent_used_page_[extent_id] = bitmap->GetAllocatedPage();
  // 将bitmap_page写回磁盘，否则下次调用时数据丢失
  WritePhysicalPage(bitmap_page, page_data);
}

bool DiskManager::IsPageFree(page_id_t logical_page_id) {
  // 根据逻辑页id得到extent以及offset
  uint32_t extent_id = logical_page_id / BITMAP_SIZE;
  uint32_t offset = logical_page_id % BITMAP_SIZE;
  uint32_t bitmap_page = extent_id * BITMAP_SIZE + 1;
  // 从磁盘中读出对应的bitmap_page
  char page_data[PAGE_SIZE];
  ReadPhysicalPage(bitmap_page, page_data);
  BitmapPage<PAGE_SIZE> *bitmap = reinterpret_cast<BitmapPage<PAGE_SIZE> *>(page_data);
  return bitmap->IsPageFree(offset);
}

page_id_t DiskManager::MapPageId(page_id_t logical_page_id) {
  uint32_t extent_id = logical_page_id / BITMAP_SIZE;
  uint32_t offset = logical_page_id % BITMAP_SIZE;
  return (BITMAP_SIZE + 1) * extent_id + offset + 2;
}

int DiskManager::GetFileSize(const std::string &file_name) {
  struct stat stat_buf;
  int rc = stat(file_name.c_str(), &stat_buf);
  return rc == 0 ? stat_buf.st_size : -1;
}

void DiskManager::ReadPhysicalPage(page_id_t physical_page_id, char *page_data) {
  int offset = physical_page_id * PAGE_SIZE;
  // check if read beyond file length
  if (offset >= GetFileSize(file_name_)) {
#ifdef ENABLE_BPM_DEBUG
    LOG(INFO) << "Read less than a page" << std::endl;
#endif
    memset(page_data, 0, PAGE_SIZE);
  } else {
    // set read cursor to offset
    db_io_.seekp(offset);
    db_io_.read(page_data, PAGE_SIZE);
    // if file ends before reading PAGE_SIZE
    int read_count = db_io_.gcount();
    if (read_count < PAGE_SIZE) {
#ifdef ENABLE_BPM_DEBUG
      LOG(INFO) << "Read less than a page" << std::endl;
#endif
      memset(page_data + read_count, 0, PAGE_SIZE - read_count);
    }
  }
}

void DiskManager::WritePhysicalPage(page_id_t physical_page_id, const char *page_data) {
  size_t offset = static_cast<size_t>(physical_page_id) * PAGE_SIZE;
  // set write cursor to offset
  db_io_.seekp(offset);
  db_io_.write(page_data, PAGE_SIZE);
  // check for I/O error
  if (db_io_.bad()) {
    LOG(ERROR) << "I/O error while writing";
    return;
  }
  // needs to flush to keep disk file in sync
  db_io_.flush();
}