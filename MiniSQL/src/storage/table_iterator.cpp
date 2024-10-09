#include "common/macros.h"
#include "storage/table_iterator.h"
#include "storage/table_heap.h"

TableIterator::TableIterator(TableHeap *tableHeap, RowId rowId, Transaction *txn){
  this->tableHeap = tableHeap;
  row = new Row(rowId);
  this->txn = txn;
  if(rowId.GetPageId() != INVALID_PAGE_ID){
    tableHeap->GetTuple(row, txn);
  }
}

TableIterator::TableIterator(const TableIterator &other) {
  tableHeap = other.tableHeap;
  row = new Row(other.row->GetRowId());
  txn = other.txn;
}

TableIterator::~TableIterator() = default;

bool TableIterator::operator==(const TableIterator &itr) const {
  return (row->GetRowId() == itr.row->GetRowId());
}

bool TableIterator::operator!=(const TableIterator &itr) const {
  return !(row->GetRowId() == itr.row->GetRowId());
}

Row &TableIterator::operator*() {
  ASSERT(row != nullptr, "row cannot be empty");
  return *row;
}

Row *TableIterator::operator->() {
  return row;
}

TableIterator &TableIterator::operator++() {
  RowId rowId = row->GetRowId();
  BufferPoolManager *bufferPoolManager = tableHeap->buffer_pool_manager_;
  // 找到page_id对应的TablePage
  TablePage *cur_Page = reinterpret_cast<TablePage *>(bufferPoolManager->FetchPage(rowId.GetPageId()));
  RowId rowId_next;
  // 寻找下一个row(tuple)
  bool flag = cur_Page->GetNextTupleRid(rowId, &rowId_next);
  if(!flag){ // 已经到该页的末尾，遍历链表
    while (cur_Page->GetNextPageId() != INVALID_PAGE_ID){
      // 找到链表下一个page_id对应的TablePage
      TablePage *next_Page = reinterpret_cast<TablePage *>(bufferPoolManager->FetchPage(cur_Page->GetNextPageId()));
      bufferPoolManager->UnpinPage(cur_Page->GetPageId(), false); // UnPin之前的页
      bool found = next_Page->GetFirstTupleRid(&rowId_next); // 寻找下一页中的Row
      if(found){
        break;
      }
      cur_Page = next_Page;
    }
  }
  row->SetRowId(rowId_next); // 更新RowId
  if(*this != tableHeap->End()){
    tableHeap->GetTuple(row, txn);
  }
  bufferPoolManager->UnpinPage(cur_Page->GetPageId(), false);
  return *this;
}

TableIterator TableIterator::operator++(int) {
  TableIterator prev(*this);
  ++(*this);
  return prev;
}
