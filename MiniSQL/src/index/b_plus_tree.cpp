#include <string>
#include "glog/logging.h"
#include "index/b_plus_tree.h"
#include "index/basic_comparator.h"
#include "index/generic_key.h"
#include "page/index_roots_page.h"

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(index_id_t index_id, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                          int leaf_max_size, int internal_max_size)
        : index_id_(index_id),
          buffer_pool_manager_(buffer_pool_manager),
          comparator_(comparator),
          leaf_max_size_(leaf_max_size),
          internal_max_size_(internal_max_size) {
  auto page = buffer_pool_manager_->FetchPage(INDEX_ROOTS_PAGE_ID);
  auto node = reinterpret_cast<IndexRootsPage*>(page->GetData());
  page_id_t root_id;
  if(node->GetRootId(index_id_, &root_id)){
      root_page_id_ = root_id;
  }
  else{
    root_page_id_ = INVALID_PAGE_ID;
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Destroy() {
}

/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::IsEmpty() const {
  return root_page_id_ == INVALID_PAGE_ID;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> &result, Transaction *transaction) {
  if(IsEmpty()){
    return false;
  }
  auto page = buffer_pool_manager_->FetchPage(root_page_id_);
  auto node = reinterpret_cast<InternalPage*>(page->GetData());
  while(!node->IsLeafPage()){
    auto value = node->Lookup(key, comparator_);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
    page = buffer_pool_manager_->FetchPage(value);
    node = reinterpret_cast<InternalPage*>(page->GetData());
  }
  // 此时node已经是叶子节点
  auto leaf = (LeafPage*)node;
  ValueType valueType;
  bool flag = leaf->Lookup(key , valueType, comparator_);
  if(flag)
    result.push_back(valueType);
  buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  return flag;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *transaction) {
  if(IsEmpty()){
    StartNewTree(key, value);
    return true;
  }
  return InsertIntoLeaf(key, value);
}
/*
 * Insert constant key & value pair into an empty tree
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
  page_id_t pageId;
  auto root = buffer_pool_manager_->NewPage(pageId);
  if(root == nullptr){
    throw std::runtime_error("out of memory");
  }
  auto node = reinterpret_cast<LeafPage*>(root->GetData());
  node->Init(pageId);
  node->Insert(key, value, comparator_);
  root_page_id_ = pageId;
  UpdateRootPageId(1);
  buffer_pool_manager_->UnpinPage(pageId, true);
}

/*
 * Insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If exist, return
 * immediately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction) {
  auto page = buffer_pool_manager_->FetchPage(root_page_id_);
  auto node = reinterpret_cast<InternalPage *>(page->GetData());
  // 和查找一样，寻找要插入的叶子节点
  while(!node->IsLeafPage()){
    auto v = node->Lookup(key, comparator_);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
    page = buffer_pool_manager_->FetchPage(v);
    node = reinterpret_cast<InternalPage*>(page->GetData());
  }
  auto leaf = (LeafPage*)node;
  ValueType valueType;
  // 如果要插入的key已存在
  bool flag = leaf->Lookup(key, valueType, comparator_);
  if(flag){
    return false;
  }
  int size = leaf->Insert(key, value, comparator_);
  if(size > leaf->GetMaxSize()){
    KeyType key_ = leaf->KeyAt(leaf->GetMinSize());
    InsertIntoParent(leaf, key_, Split(leaf));
  }
  buffer_pool_manager_->UnpinPage(leaf->GetPageId(), true);
  return true;
}

/*
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 */
INDEX_TEMPLATE_ARGUMENTS
template<typename N>
N *BPLUSTREE_TYPE::Split(N *node) {
  page_id_t pageId;
  auto page = buffer_pool_manager_->NewPage(pageId);
  N* new_node;
  if(page == nullptr){
    throw std::runtime_error("out of memory");
  }
  auto old = (BPlusTreePage*)node;
  if(old->IsLeafPage()){
    auto new_leaf = reinterpret_cast<LeafPage *>(page->GetData());
    new_leaf->Init(pageId);
    auto old_leaf = (LeafPage*)node;
    old_leaf->MoveHalfTo(new_leaf);
    new_leaf->SetNextPageId(old_leaf->GetNextPageId());
    old_leaf->SetNextPageId(new_leaf->GetPageId());
//    InsertIntoParent(old_leaf, new_leaf->KeyAt(0),new_leaf);
    new_node = reinterpret_cast<N*>(new_leaf);
//    buffer_pool_manager_->UnpinPage(pageId, true);
  }
  else{
    auto new_internal = reinterpret_cast<InternalPage *>(page->GetData());
    new_internal->Init(pageId);
    auto old_internal = (InternalPage*)node;
    old_internal->MoveHalfTo(new_internal, buffer_pool_manager_);
//    InsertIntoParent(old_internal, new_internal->KeyAt(0), new_internal);
//    buffer_pool_manager_->UnpinPage(pageId, true);
    new_node = reinterpret_cast<N*>(new_internal);
  }
  return new_node; // 此时不能Unpin，接下来还需要完成InsertIntoParent
}

/*
 * Insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                                      Transaction *transaction) {
  if(old_node->IsRootPage()){
    page_id_t pageId;
    auto page = buffer_pool_manager_->NewPage(pageId);
    auto root = reinterpret_cast<InternalPage*>(page->GetData());
    root->Init(pageId);
    root->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    old_node->SetParentPageId(pageId);
    new_node->SetParentPageId(pageId);
    root_page_id_ = pageId;
    UpdateRootPageId();
    // 有可能重复unpin了，但修改了UnpinPage的代码，使pin_count不能小于0
    buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
    return;
  }
  page_id_t parent_page_id = old_node->GetParentPageId();
  auto page = buffer_pool_manager_->FetchPage(parent_page_id);
  auto node = reinterpret_cast<InternalPage*>(page->GetData());
  int size = node->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
  old_node->SetParentPageId(parent_page_id);
  new_node->SetParentPageId(parent_page_id);
  if(size > node->GetMaxSize()){
    KeyType key_ = node->KeyAt(node->GetMinSize());
    InsertIntoParent(node, key_,Split(node));
  }
  buffer_pool_manager_->UnpinPage(parent_page_id, true);
  buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immediately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {
  if(IsEmpty()){
    return;
  }
  auto page = buffer_pool_manager_->FetchPage(root_page_id_);
  auto node = reinterpret_cast<InternalPage *>(page->GetData());
  // 和查找一样，寻找要插入的叶子节点
  while(!node->IsLeafPage()){
    auto v = node->Lookup(key, comparator_);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
    page = buffer_pool_manager_->FetchPage(v);
    node = reinterpret_cast<InternalPage*>(page->GetData());
  }
  auto leaf = (LeafPage*)node;
  ValueType valueType;
  // 如果要删除的key不存在，直接返回
  bool flag = leaf->Lookup(key, valueType, comparator_);
  if(!flag){
    return;
  }
  int old_size = leaf->GetSize();
  int size = leaf->RemoveAndDeleteRecord(key, comparator_);
  if(old_size == size){
    return;
  }
//  if(leaf->IsRootPage()){
//    return;
//  }
  bool delete_ = CoalesceOrRedistribute(leaf);
  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
  if(delete_) {
    buffer_pool_manager_->DeletePage(page->GetPageId());
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
}

/*
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
INDEX_TEMPLATE_ARGUMENTS
template<typename N>
bool BPLUSTREE_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) {
  if(node->IsRootPage()){
    return AdjustRoot(node);
  }
  if(node->GetSize() >= node->GetMinSize()){
    return false;
  }
  page_id_t pageId = node->GetParentPageId();
  auto page = buffer_pool_manager_->FetchPage(pageId);
  auto parent_node = reinterpret_cast<InternalPage*>(page->GetData());
  // 找到左右sibling
  int index = parent_node->ValueIndex(node->GetPageId());
  bool deleteOrNot;
  int sibling_index = index == 0 ? 1 : index - 1;
  page_id_t sibling_pageId = parent_node->ValueAt(sibling_index);
  auto sibling_page = buffer_pool_manager_->FetchPage(sibling_pageId);
  auto sibling_node = reinterpret_cast<N*>(sibling_page->GetData());

  if(sibling_node->GetSize() + node->GetSize() > node->GetMaxSize()){
    Redistribute(sibling_node, node, index);
    buffer_pool_manager_->UnpinPage(parent_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(sibling_node->GetPageId(), true);
    return false;
  }
  else{
    deleteOrNot = Coalesce(&sibling_node, &node, &parent_node, index);
    buffer_pool_manager_->UnpinPage(parent_node->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(sibling_node->GetPageId(), true);
    if(deleteOrNot){ // parent_node应该删除
      buffer_pool_manager_->DeletePage(parent_node->GetPageId());
    }
    return true; // node需要被删除
  }

  return false;
}

/*
 * Move all the key & value pairs from one page to its sibling page, and notify
 * buffer pool manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @param   index              index==0表示neighbor在node的右边，否则在左边
 * @return  true means parent node should be deleted, false means no deletion happened
 */
INDEX_TEMPLATE_ARGUMENTS
template<typename N>
bool BPLUSTREE_TYPE::Coalesce(N **neighbor_node, N **node,
                              BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent, int index,
                              Transaction *transaction) {
  int index_ = index;
  if(index == 0){
    index_ = 1;
    std::swap(neighbor_node, node);
  }
  KeyType key = (*parent)->KeyAt(index_);

  auto node_ = (BPlusTreePage*)(*node);
  if(node_->IsLeafPage()){
    auto leaf = (LeafPage*)(*node);
    auto sibling_leaf = (LeafPage*)(*neighbor_node);
    leaf->MoveAllTo(sibling_leaf);
    (*parent)->Remove(index_);
  }
  else{
    auto internal = (InternalPage*)(*node);
    auto sibling_internal = (InternalPage*)(*neighbor_node);
    internal->MoveAllTo(sibling_internal, key, buffer_pool_manager_);
    (*parent)->Remove(index_);
  }
  return CoalesceOrRedistribute(*parent);
}

/*
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
INDEX_TEMPLATE_ARGUMENTS
template<typename N>
void BPLUSTREE_TYPE::Redistribute(N *neighbor_node, N *node, int index) {
  auto page = buffer_pool_manager_->FetchPage(node->GetParentPageId());
  auto parent = reinterpret_cast<InternalPage*>(page->GetData());
  if(node->IsLeafPage()){
    auto leaf = (LeafPage*)(node);
    auto sibling_leaf = (LeafPage*)(neighbor_node);
    if(index == 0){
      sibling_leaf->MoveFirstToEndOf(leaf);
      parent->SetKeyAt(1, sibling_leaf->KeyAt(0));
    }
    else{
      sibling_leaf->MoveLastToFrontOf(leaf);
      parent->SetKeyAt(index, leaf->KeyAt(0));
    }
  }
  else{
    auto internal = (InternalPage*)(node);
    auto sibling_internal = (InternalPage*)(neighbor_node);
    KeyType key;
    if(index == 0){
      key = parent->KeyAt(1);
      sibling_internal->MoveFirstToEndOf(internal, key, buffer_pool_manager_);
      parent->SetKeyAt(1, sibling_internal->KeyAt(0));
    }
    else{
      key = parent->KeyAt(index);
      sibling_internal->MoveLastToFrontOf(internal, key, buffer_pool_manager_);
      parent->SetKeyAt(index, internal->KeyAt(0));
    }
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
}

/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happened
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::AdjustRoot(BPlusTreePage *old_root_node) {
  if(!old_root_node->IsLeafPage() && old_root_node->GetSize() == 1){
    auto internal = reinterpret_cast<InternalPage*>(old_root_node);
    page_id_t pageId = internal->RemoveAndReturnOnlyChild();
    root_page_id_ = pageId;
    UpdateRootPageId();
    auto page = buffer_pool_manager_->FetchPage(pageId);
    auto node = reinterpret_cast<BPlusTreePage*>(page->GetData());
    node->SetParentPageId(INVALID_PAGE_ID);
    buffer_pool_manager_->UnpinPage(pageId, true);
    return true;
  }
  if(old_root_node->IsLeafPage() && old_root_node->GetSize() == 0){
    root_page_id_ = INVALID_PAGE_ID;
    UpdateRootPageId();
    return true;
  }
  return false;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the left most leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin() {
  KeyType key{};
  Page* page = FindLeafPage(key, true);
  auto node = reinterpret_cast<LeafPage*>(page->GetData());
  int index = 0;
  return {node, index, buffer_pool_manager_};
}

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::Begin(const KeyType &key) {
  Page* page = FindLeafPage(key, false);
  auto node = reinterpret_cast<LeafPage*>(page->GetData());
  int index = node->KeyIndex(key, comparator_);
  return {node, index, buffer_pool_manager_};
}

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::End() {
  return {INVALID_PAGE_ID, -1};
}

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 * Note: the leaf page is pinned, you need to unpin it after use.
 */
INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREE_TYPE::FindLeafPage(const KeyType &key, bool leftMost) {
  auto page = buffer_pool_manager_->FetchPage(root_page_id_);
  auto node = reinterpret_cast<InternalPage *>(page->GetData());
  if(!leftMost) {
    while (!node->IsLeafPage()) {
      auto v = node->Lookup(key, comparator_);
      buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
      page = buffer_pool_manager_->FetchPage(v);
      node = reinterpret_cast<InternalPage *>(page->GetData());
    }
  }
  else{
    while (!node->IsLeafPage()) {
      auto v = node->ValueAt(0);
      buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
      page = buffer_pool_manager_->FetchPage(v);
      node = reinterpret_cast<InternalPage *>(page->GetData());
    }
  }
  return page;
}

/*
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      default value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
  auto page = buffer_pool_manager_->FetchPage(INDEX_ROOTS_PAGE_ID);
  auto index_node = reinterpret_cast<IndexRootsPage*>(page->GetData());
  if(insert_record == 0){
    index_node->Update(index_id_, root_page_id_);
  }
  else{
    index_node->Insert(index_id_, root_page_id_);
  }
  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(BPlusTreePage *page, BufferPoolManager *bpm, std::ofstream &out) const {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    // Print node name
    out << leaf_prefix << leaf->GetPageId();
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << leaf->GetPageId()
        << ",Parent=" << leaf->GetParentPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << leaf->GetPageId() << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << leaf->GetPageId() << " " << leaf_prefix << leaf->GetNextPageId()
          << "};\n";
    }

    // Print parent links if there is a parent
    if (leaf->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << leaf->GetParentPageId() << ":p" << leaf->GetPageId() << " -> " << leaf_prefix
          << leaf->GetPageId() << ";\n";
    }
  } else {
    auto *inner = reinterpret_cast<InternalPage *>(page);
    // Print node name
    out << internal_prefix << inner->GetPageId();
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << inner->GetPageId()
        << ",Parent=" << inner->GetParentPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Parent link
    if (inner->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << inner->GetParentPageId() << ":p" << inner->GetPageId() << " -> "
          << internal_prefix
          << inner->GetPageId() << ";\n";
    }
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(child_page, bpm, out);
      if (i > 0) {
        auto sibling_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i - 1))->GetData());
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page->GetPageId() << " " << internal_prefix
              << child_page->GetPageId() << "};\n";
        }
        bpm->UnpinPage(sibling_page->GetPageId(), false);
      }
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

/**
 * This function is for debug only, you don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    std::cout << "Leaf Page: " << leaf->GetPageId() << " parent: " << leaf->GetParentPageId()
              << " next: " << leaf->GetNextPageId() << std::endl;
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  } else {
    auto *internal = reinterpret_cast<InternalPage *>(page);
    std::cout << "Internal Page: " << internal->GetPageId() << " parent: " << internal->GetParentPageId()
              << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(internal->ValueAt(i))->GetData()), bpm);
      bpm->UnpinPage(internal->ValueAt(i), false);
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::Check() {
  bool all_unpinned = buffer_pool_manager_->CheckAllUnpinned();
  if (!all_unpinned) {
    LOG(ERROR) << "problem in page unpin" << endl;
  }
  return all_unpinned;
}

template
class BPlusTree<int, int, BasicComparator<int>>;

template
class BPlusTree<GenericKey<4>, RowId, GenericComparator<4>>;

template
class BPlusTree<GenericKey<8>, RowId, GenericComparator<8>>;

template
class BPlusTree<GenericKey<16>, RowId, GenericComparator<16>>;

template
class BPlusTree<GenericKey<32>, RowId, GenericComparator<32>>;

template
class BPlusTree<GenericKey<64>, RowId, GenericComparator<64>>;
