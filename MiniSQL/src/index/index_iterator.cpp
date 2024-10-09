#include "index/basic_comparator.h"
#include "index/generic_key.h"
#include "index/index_iterator.h"

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE::IndexIterator(B_PLUS_TREE_LEAF_PAGE_TYPE* leaf, int index, BufferPoolManager* bufferPoolManager) {
  this->leaf = leaf;
  this->index = index;
  buffer_pool_manager_ = bufferPoolManager;
  page_id_ = leaf->GetPageId();
}

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE::IndexIterator(page_id_t pageId, int index) {
  page_id_ = pageId;
  this->index = index;
}

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE::~IndexIterator() {

}

INDEX_TEMPLATE_ARGUMENTS const MappingType &INDEXITERATOR_TYPE::operator*() {
  return leaf->GetItem(index);
}

INDEX_TEMPLATE_ARGUMENTS INDEXITERATOR_TYPE &INDEXITERATOR_TYPE::operator++() {
  int size = leaf->GetSize();
  index++;
  if (index < size){
    return *this;
  }
  page_id_t pageId = leaf->GetNextPageId();
  buffer_pool_manager_->UnpinPage(leaf->GetPageId(), false);
  if(pageId == INVALID_PAGE_ID){
    index = -1;
    page_id_ = INVALID_PAGE_ID;
    leaf = nullptr;
    return *this;
  }
  auto page = buffer_pool_manager_->FetchPage(pageId);
  auto new_leaf = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE*>(page->GetData());
  leaf = new_leaf;
  index = 0;
  page_id_ = pageId;
  return *this;
}

INDEX_TEMPLATE_ARGUMENTS
bool INDEXITERATOR_TYPE::operator==(const IndexIterator &itr) const {
  return page_id_ == itr.page_id_ && index == itr.index;
}

INDEX_TEMPLATE_ARGUMENTS
bool INDEXITERATOR_TYPE::operator!=(const IndexIterator &itr) const {
  return page_id_ != itr.page_id_ || index != itr.index;
}

template
class IndexIterator<int, int, BasicComparator<int>>;

template
class IndexIterator<GenericKey<4>, RowId, GenericComparator<4>>;

template
class IndexIterator<GenericKey<8>, RowId, GenericComparator<8>>;

template
class IndexIterator<GenericKey<16>, RowId, GenericComparator<16>>;

template
class IndexIterator<GenericKey<32>, RowId, GenericComparator<32>>;

template
class IndexIterator<GenericKey<64>, RowId, GenericComparator<64>>;
