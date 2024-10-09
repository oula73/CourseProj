#include "buffer/buffer_pool_manager.h"
#include "glog/logging.h"
#include "page/bitmap_page.h"

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager)
        : pool_size_(pool_size), disk_manager_(disk_manager) {
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size_);
  for (size_t i = 0; i < pool_size_; i++) {
    free_list_.emplace_back(i);
  }
}

BufferPoolManager::~BufferPoolManager() {
  // 在FlushPage()中会对page_table进行改动，测试时会陷入死循环，因此不能用此方法
//  for (auto page: page_table_) {
//    FlushPage(page.first);
//  }
  for(size_t i = 0; i < pool_size_; i++){
    page_id_t pageId = pages_[i].GetPageId();
    if(pageId != INVALID_PAGE_ID){
      FlushPage(pageId);
    }
  }
  delete[] pages_;
  delete replacer_;
}

Page *BufferPoolManager::FetchPage(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  frame_id_t frameId;
  page_id_t pageId;
  if(page_table_.find(page_id) != page_table_.end()){ // 在buffer_pool中
    // 取到对应的buffer_pool中的id
    frameId = page_table_[page_id];
    // 更新pin_count
    pages_[frameId].pin_count_++;
    // 从lru替换列表中移除，防止被移出buffer_pool
    replacer_->Pin(frameId);
    return &pages_[frameId];
  }
  else{ // 不在buffer_pool中
    // 从free_list中找到一个新page
    if(!free_list_.empty()){
      // 找到frame_id
      frameId = free_list_.back();
      free_list_.pop_back();
      // 更新page_table_中的映射信息
      page_table_.insert({page_id, frameId});
      // 更新page对象中的meta信息
      pages_[frameId].page_id_ = page_id;
      pages_[frameId].pin_count_ = 1;
      pages_[frameId].is_dirty_ = false;
      disk_manager_->ReadPage(page_id, pages_[frameId].GetData());
      //replacer_->Pin(frameId);// 应该是多余的吧
      return &pages_[frameId];
    }
    else{
      if(replacer_->Victim(&frameId)){
        // 找到frame_id对应的page_id
        for(auto itr : page_table_){
          if(itr.second == frameId){
            pageId = itr.first;
            break;
          }
        }
        // 如果有改动，需要写入到磁盘中
        if(pages_[frameId].IsDirty()){
          disk_manager_->WritePage(pageId, pages_[frameId].GetData());
        }
        // 更新page_table中的映射关系
        page_table_.erase(pageId);
        page_table_.insert({page_id, frameId});
        // 更新page对象的meta信息
        pages_[frameId].page_id_ = page_id;
        pages_[frameId].pin_count_ = 1;
        pages_[frameId].is_dirty_ = false;
        // 从磁盘中读出page_id对应的内容
        disk_manager_->ReadPage(page_id, pages_[frameId].GetData());
        return &pages_[frameId];
      }
      else{
        return nullptr;
      }
    }
  }
  return nullptr;
}

// NewPage的实现过程与FetchPage几乎相同，除了NewPage是将当前页的内容清空
// 而FetchPage是要从磁盘中读入
Page *BufferPoolManager::NewPage(page_id_t &page_id) {
  // 0.   Make sure you call AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  frame_id_t frameId;
  if(free_list_.empty() && replacer_->Size() == 0){
    return nullptr;
  }
  page_id = AllocatePage();
  if(!free_list_.empty()){
    frameId = free_list_.back();
    free_list_.pop_back();
    page_table_.insert({page_id, frameId});

    pages_[frameId].page_id_ = page_id;
    pages_[frameId].ResetMemory();
    pages_[frameId].pin_count_ = 1;
    pages_[frameId].is_dirty_ = false;

    return &pages_[frameId];
  }
  else{
    page_id_t pageId;
    replacer_->Victim(&frameId);

    for(auto itr : page_table_){
      if(itr.second == frameId){
        pageId = itr.first;
        break;
      }
    }

    if(pages_[frameId].IsDirty()){
      disk_manager_->WritePage(pageId, pages_[frameId].GetData());
    }

    page_table_.erase(pageId);
    page_table_.insert({page_id, frameId});

    pages_[frameId].page_id_ = page_id;
    pages_[frameId].ResetMemory();
    pages_[frameId].pin_count_ = 1;
    pages_[frameId].is_dirty_ = false;

    return &pages_[frameId];
  }

  return nullptr;
}

bool BufferPoolManager::DeletePage(page_id_t page_id) {
  // 0.   Make sure you call DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  DeallocatePage(page_id);
  frame_id_t frameId;
  if(page_table_.find(page_id) == page_table_.end()){
    return true;
  }
  else{
    frameId = page_table_[page_id];
    if(pages_[frameId].pin_count_ != 0){
      return false;
    }
    else{
//      if(pages_[frameId].IsDirty()){ // 写回到磁盘中
//        disk_manager_->WritePage(page_id, pages_[frameId].GetData());
//      }
      // remove from the page table
      page_table_.erase(page_id);
      // return it to the free list
      free_list_.emplace_back(frameId);
      // reset its metadata
      pages_[frameId].is_dirty_ = false;
      pages_[frameId].ResetMemory();
      pages_[frameId].page_id_ = INVALID_PAGE_ID;
      return true;
    }
  }
  return false;
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
  if(page_table_.find(page_id) == page_table_.end())
    return false;
  frame_id_t frameId = page_table_[page_id];
  pages_[frameId].pin_count_ = 0;
//  if(pages_[frameId].pin_count_ > 0)
//    pages_[frameId].pin_count_--;
//  if(pages_[frameId].pin_count_ == 0) // 不能直接unpin, 要判断pin_count是否为0，但是单线程情况下好像不需要考虑这些
    replacer_->Unpin(frameId);
  pages_[frameId].is_dirty_ |= is_dirty; // !!!注意要用或，例如可能之前已经是true了，而is_dirty为false，这样就不能直接赋值了
  return true;
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
  if(page_id == INVALID_PAGE_ID)
    return false;
  if(page_table_.find(page_id) == page_table_.end()){
    return false;
  }
  frame_id_t frameId = page_table_[page_id];
  if(pages_[frameId].IsDirty()){
    disk_manager_->WritePage(page_id, pages_[frameId].GetData());
  }
  pages_[frameId].pin_count_ = 0;
  //pages_[frameId].page_id_ = INVALID_PAGE_ID;
  pages_[frameId].is_dirty_ = false;
  //pages_[frameId].ResetMemory();
  page_table_.erase(page_id);
  free_list_.emplace_back(frameId); // 记得返回到free_list_中
  return true;
}

page_id_t BufferPoolManager::AllocatePage() {
  int next_page_id = disk_manager_->AllocatePage();
  return next_page_id;
}

void BufferPoolManager::DeallocatePage(page_id_t page_id) {
  disk_manager_->DeAllocatePage(page_id);
}

bool BufferPoolManager::IsPageFree(page_id_t page_id) {
  return disk_manager_->IsPageFree(page_id);
}

// Only used for debug
bool BufferPoolManager::CheckAllUnpinned() {
  bool res = true;
  for (size_t i = 0; i < pool_size_; i++) {
    if (pages_[i].pin_count_ != 0) {
      res = false;
      LOG(ERROR) << "page " << pages_[i].page_id_ << " pin count:" << pages_[i].pin_count_ << endl;
    }
  }
  return res;
}