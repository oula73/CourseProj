#include "page/b_plus_tree_page.h"

/*
 * Helper methods to get/set page type
 * Page type enum class is defined in b_plus_tree_page.h
 */
bool BPlusTreePage::IsLeafPage() const {
  return page_type_ == IndexPageType::LEAF_PAGE;
}

bool BPlusTreePage::IsRootPage() const {
  return parent_page_id_ == INVALID_PAGE_ID;
}

void BPlusTreePage::SetPageType(IndexPageType page_type) {
  page_type_ = page_type;
}

/*
 * Helper methods to get/set size (number of key/value pairs stored in that
 * page)
 */
int BPlusTreePage::GetSize() const {
  return size_;
}

void BPlusTreePage::SetSize(int size) {
  size_ = size;
}

void BPlusTreePage::IncreaseSize(int amount) {
  size_ += amount;
}

/*
 * Helper methods to get/set max size (capacity) of the page
 */
int BPlusTreePage::GetMaxSize() const {
  return max_size_;
}

void BPlusTreePage::SetMaxSize(int size) {
  max_size_ = size;
}

/*
 * Helper method to get min page size
 * Generally, min page size == max page size / 2
 */
int BPlusTreePage::GetMinSize() const {
//  if(IsRootPage()){ // 是根节点
//    if(!IsLeafPage()) // 不是叶子节点
//      return 2;
//    else // 是叶子节点，说明此时B+树只有一个page，初始最小值为1
//      return 1;
//  }
//  else{
    //return static_cast<int>(ceil(max_size_ / 2)); // 因为size是pointer的数量，向上取整即可
//  }
    return max_size_ / 2;
}

/*
 * Helper methods to get/set parent page id
 */
page_id_t BPlusTreePage::GetParentPageId() const {
  return parent_page_id_;
}

void BPlusTreePage::SetParentPageId(page_id_t parent_page_id) {
  parent_page_id_ = parent_page_id;
}

/*
 * Helper methods to get/set self page id
 */
page_id_t BPlusTreePage::GetPageId() const {
  return page_id_;
}

void BPlusTreePage::SetPageId(page_id_t page_id) {
  page_id_ = page_id;
}

/*
 * Helper methods to set lsn
 */
void BPlusTreePage::SetLSN(lsn_t lsn) {
  lsn_ = lsn;
}