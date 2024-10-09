#include "page/bitmap_page.h"

/**
 * 位图中每个比特（Bit）对应一个数据页的分配情况，用于标记该数据页是否空闲（ 0 表示空闲， 1 表示已分配）
 *
 * 对于AllocatePage，感觉说的不是很清楚，是可以随机分配还是按顺序呢？如果按顺序的话，中途DeAllocatePage的话则需要
 * 从头遍历来选择合适的page_offset，这样的话复杂度挺高而且next_free_page_似乎没有存在的意义，还没有想出比较好的实
 * 现方式，先暴力求解吧
 * （一开始尝试维护free_page，但测试代码直接转化为类指针，绕过了构造器，free_page没地方初始化）
 */

//template<size_t PageSize>
//BitmapPage<PageSize>::BitmapPage() {
//  // 初始化free_page
//  for(uint32_t i = 0; i < MAX_CHARS; i++){
//    free_page.insert(i);
//  }
//}

template<size_t PageSize>
bool BitmapPage<PageSize>::AllocatePage(uint32_t &page_offset) {
//  if(free_page.empty())
//    return false;
  // 判断是否已经分配满
  if(page_allocated_ == 8 * MAX_CHARS)
    return false;
  // 如果相等，则说明next_free_page_之前的下标全被使用了
  if(page_allocated_ == next_free_page_){
    page_offset = next_free_page_;
    page_allocated_++;
    next_free_page_++;
  }// 不相等，说明之前有DeAllocatePage
  else {
    size_t i;
    // 有更好的解决方法吗？ 待完善
    for (i = 0; i <= page_allocated_; i++) {
      if (bytes[i] != 255) break;
    }
    int j;
    for (j = 0; j < 8; j++) {
      if (!(bytes[i] >> j & 1)) break;
    }
    page_offset = i * 8 + j;
    page_allocated_++;
  }
  uint32_t byte_index = page_offset / 8;
  uint8_t bit_index = page_offset % 8;
  bytes[byte_index] = bytes[byte_index] | (1 << bit_index);
//  std::bitset<8> bs(bytes[byte_index]);
//  bytes[byte_index] = reinterpret_cast<uint32_t>(bs.set(bit_index).to_ulong());
//  free_page.erase(page_offset);
//  next_free_page_ = *(free_page.begin());
  return true;
}

template<size_t PageSize>
bool BitmapPage<PageSize>::DeAllocatePage(uint32_t page_offset) {
  if(IsPageFree(page_offset))
    return false;
  uint32_t byte_index = page_offset / 8;
  uint8_t bit_index = page_offset % 8;
//  std::bitset<8> bs(bytes[byte_index]);
//  bs.reset(bit_index);
  bytes[byte_index] = bytes[byte_index] & ~(1 << bit_index);
  page_allocated_--;
//  free_page.insert(page_offset);
//  next_free_page_ = *(free_page.begin());
  return true;
}

template<size_t PageSize>
bool BitmapPage<PageSize>::IsPageFree(uint32_t page_offset) const {
  return IsPageFreeLow(page_offset / 8, page_offset % 8);
}

template<size_t PageSize>
bool BitmapPage<PageSize>::IsPageFreeLow(uint32_t byte_index, uint8_t bit_index) const {
  // 找出具体位的值
  std::bitset<8> bs(bytes[byte_index]);
  return !bs.test(bit_index);
}

template
class BitmapPage<64>;

template
class BitmapPage<128>;

template
class BitmapPage<256>;

template
class BitmapPage<512>;

template
class BitmapPage<1024>;

template
class BitmapPage<2048>;

template
class BitmapPage<4096>;