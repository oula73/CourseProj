#include "buffer/lru_replacer.h"

LRUReplacer::LRUReplacer(size_t num_pages) {
  frame_num = num_pages;
}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  if(Size() == 0)
    return false;
  frame_id_t frameId = lru_list_.back();
  *frame_id = frameId;
  lru_list_.pop_back();
  lruList.erase(frameId);
  buffer.erase(frameId);
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  lru_list_.remove(frame_id);
  lruList.erase(frame_id);
}

/**
 * 将数据页解除固定，放入 lru_list_ 中
 * 有个疑惑的点，为什么再次UnPin到重复的元素不用将其移到双向链表的头部呢？
 */
void LRUReplacer::Unpin(frame_id_t frame_id) {
  if(buffer.find(frame_id) == buffer.end()){
//    if(buffer.size() < frame_num) {
      buffer.insert(frame_id);
      lru_list_.emplace_front(frame_id);
      lruList.insert(frame_id);
//    } // 不需要在这里处理
//    else{     //容量不足，需要移除元素
//      frame_id_t frameId;
//      Victim(&frameId);// 如果返回false该如何处理？
//      buffer.insert(frame_id);
//      lru_list_.emplace_front(frame_id);
//    }
  }
  else{
    // ???
    if(lruList.find(frame_id) == lruList.end()){
      lru_list_.emplace_front(frame_id);
      lruList.insert(frame_id);
    }
  }
}

size_t LRUReplacer::Size() {
  return lru_list_.size();
}