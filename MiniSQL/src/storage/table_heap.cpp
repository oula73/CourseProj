#include "storage/table_heap.h"

bool TableHeap::InsertTuple(Row &row, Transaction *txn) {
//  RowId rid = row.GetRowId();
  // 从first_page开始寻找插入的位置
  auto cur_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(insert_page_id_));
  if(cur_page == nullptr){
    return false;
  }
  // 如果空间不足，按照链表顺序转到下一页
  while(!cur_page->InsertTuple(row, schema_, txn, lock_manager_, log_manager_)){
    auto next_page_id = cur_page->GetNextPageId();
    if(next_page_id != INVALID_PAGE_ID){
      buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), false);
      // 获取下一页的page
      cur_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(next_page_id));
    }
    else{ // 如果下一页是双向链表的结尾的话，新建一个page
      auto new_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(next_page_id));
      if(new_page == nullptr){ // 没有空间新建，返回false
        buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), false);
        return false;
      }
      // 将new_page添加到双向链表中
      cur_page->SetNextPageId(next_page_id);
      new_page->Init(next_page_id, cur_page->GetTablePageId(), log_manager_, txn);
      buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), true);
      cur_page = new_page;
    }
  }
  insert_page_id_ = cur_page->GetPageId();
  buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), true);
  return true;
}

bool TableHeap::MarkDelete(const RowId &rid, Transaction *txn) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  // If the page could not be found, then abort the transaction.
  if (page == nullptr) {
    return false;
  }
  // Otherwise, mark the tuple as deleted.
  page->WLatch();
  page->MarkDelete(rid, txn, lock_manager_, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
  return true;
}

bool TableHeap::UpdateTuple(Row &row, const RowId &rid, Transaction *txn) {
  auto cur_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  int status; // 记录返回的状态
  status = cur_page->UpdateTuple(row, new Row(rid), schema_, txn, lock_manager_, log_manager_);
  if(status == 1){ // 更新成功
    buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), true);
    return true;
  }
  if(status != -2){ // 除空间不足外的其他原因
    buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), false);
    return false;
  }
  // 更新后空间不足
  // 在cur_page中删除原来的row
  cur_page->MarkDelete(rid, txn, lock_manager_, log_manager_);
  buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), false);
  // 寻找新的页进行插入
  return InsertTuple(row, txn);
}

void TableHeap::ApplyDelete(const RowId &rid, Transaction *txn) {
  // Step1: Find the page which contains the tuple.
  // Step2: Delete the tuple from the page.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  page->ApplyDelete(rid, txn, log_manager_);
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
}

void TableHeap::RollbackDelete(const RowId &rid, Transaction *txn) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  assert(page != nullptr);
  // Rollback the delete.
  page->WLatch();
  page->RollbackDelete(rid, txn, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
}

void TableHeap::FreeHeap() {
  page_id_t cur_page_id = first_page_id_;
  while(cur_page_id != INVALID_PAGE_ID){
    auto cur_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(first_page_id_));
    page_id_t next_page_id = cur_page->GetNextPageId();
    buffer_pool_manager_->UnpinPage(cur_page_id, false);
    buffer_pool_manager_->DeletePage(cur_page_id);
    cur_page_id = next_page_id;
  }
}

bool TableHeap::GetTuple(Row *row, Transaction *txn) {
  RowId rowId = row->GetRowId();
  auto* tablePage = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rowId.GetPageId()));
  if(tablePage == nullptr)
    return false;
  tablePage->GetTuple(row, schema_, txn, lock_manager_);
  return true;
}

TableIterator TableHeap::Begin(Transaction *txn) {
  page_id_t pageId = first_page_id_;
  // 从first_page开始遍历链表找到第一条记录
  RowId first_rid;
  while(pageId != INVALID_PAGE_ID) {
    auto *tablePage = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(pageId));
    auto found = tablePage->GetFirstTupleRid(&first_rid);
    buffer_pool_manager_->UnpinPage(tablePage->GetPageId(), false);
    if(found){
      break;
    }
    pageId = tablePage->GetNextPageId();
  }
  return TableIterator(this, first_rid, txn);
}

TableIterator TableHeap::End() {
  return TableIterator(this, INVALID_ROWID, nullptr);
}
