#include "catalog/catalog.h"

void CatalogMeta::SerializeTo(char *buf) const {
  // ASSERT(false, "Not Implemented yet");
  uint32_t len = 0;
  MACH_WRITE_UINT32(buf, CATALOG_METADATA_MAGIC_NUM);
  len += sizeof(uint32_t);

  uint32_t size1 = table_meta_pages_.size();
  MACH_WRITE_UINT32(buf + len, size1);
  len += sizeof(uint32_t);

  for(auto pair : table_meta_pages_){
    MACH_WRITE_UINT32(buf + len, pair.first);
    len += sizeof(table_id_t);
    MACH_WRITE_INT32(buf + len, pair.second);
    len += sizeof(page_id_t);
  }

  uint32_t size2 = index_meta_pages_.size();
  MACH_WRITE_UINT32(buf + len, size2);
  len += sizeof(uint32_t);

  for(auto pair : index_meta_pages_){
    MACH_WRITE_UINT32(buf + len, pair.first);
    len += sizeof(index_id_t);
    MACH_WRITE_INT32(buf + len, pair.second);
    len += sizeof(page_id_t);
  }
}

CatalogMeta *CatalogMeta::DeserializeFrom(char *buf, MemHeap *heap) {
  // ASSERT(false, "Not Implemented yet");
  uint32_t len = 0;
  uint32_t magic = MACH_READ_UINT32(buf);
  ASSERT(magic == CATALOG_METADATA_MAGIC_NUM, "wrong catalog serialize");
  len += sizeof(uint32_t);

  uint32_t size1 = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  std::map<table_id_t, page_id_t> table_;
  for(uint32_t i = 0; i < size1; i++){
    table_id_t tableId = MACH_READ_UINT32(buf + len);
    len += sizeof(table_id_t);
    page_id_t pageId = MACH_READ_UINT32(buf + len);
    len += sizeof(page_id_t);
    table_.insert({tableId, pageId});
  }

  uint32_t size2 = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  std::map<index_id_t, page_id_t> index_;
  for(uint32_t i = 0; i < size2; i++){
    index_id_t indexId = MACH_READ_UINT32(buf + len);
    len += sizeof(index_id_t);
    page_id_t pageId1 = MACH_READ_UINT32(buf + len);
    len += sizeof(page_id_t);
    index_.insert({indexId, pageId1});
  }

  void* mem = heap->Allocate(sizeof(CatalogMeta));
  CatalogMeta* catalogMeta = new(mem) CatalogMeta(table_, index_);
  return catalogMeta;
}

uint32_t CatalogMeta::GetSerializedSize() const {
  // ASSERT(false, "Not Implemented yet");
  uint32_t size1 = table_meta_pages_.size();
  uint32_t size2 = index_meta_pages_.size();
  return 3 * sizeof(uint32_t) + size1 * (sizeof(table_id_t) + sizeof(page_id_t)) + size2 * (sizeof(index_id_t) + sizeof(page_id_t));
}

CatalogMeta::CatalogMeta() {}


CatalogManager::CatalogManager(BufferPoolManager *buffer_pool_manager, LockManager *lock_manager,
                               LogManager *log_manager, bool init)
        : buffer_pool_manager_(buffer_pool_manager), lock_manager_(lock_manager),
          log_manager_(log_manager), heap_(new SimpleMemHeap()) {
  // ASSERT(false, "Not Implemented yet");
  if(init){
    catalog_meta_ = CatalogMeta::NewInstance(heap_);
    FlushCatalogMetaPage();
  }
  else{
    auto page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
    catalog_meta_ = CatalogMeta::DeserializeFrom(page->GetData(), heap_);
    buffer_pool_manager->UnpinPage(page->GetPageId(), false);
    for(auto i : catalog_meta_->table_meta_pages_){
      table_id_t tableId = i.first;
      page_id_t pageId = i.second;
      LoadTable(tableId, pageId);
    }
    for(auto i : catalog_meta_->index_meta_pages_){
      index_id_t indexId = i.first;
      page_id_t pageId = i.second;
      LoadIndex(indexId, pageId);
    }
  }
}

CatalogManager::~CatalogManager() {
  delete heap_;
}

dberr_t CatalogManager::CreateTable(const string &table_name, TableSchema *schema,
                                    Transaction *txn, TableInfo *&table_info) {
  // ASSERT(false, "Not Implemented yet");
  if(table_names_.find(table_name) != table_names_.end()){
    return DB_TABLE_ALREADY_EXIST;
  }
  table_info = TableInfo::Create(heap_);
  page_id_t pageId;
  table_id_t tableId = catalog_meta_->GetNextTableId();
//  auto page = reinterpret_cast<TablePage*>(buffer_pool_manager_->NewPage(pageId)); // 理解错了，不需要在这里新建table_page
  auto schema_ = Schema::DeepCopySchema(schema, table_info->GetMemHeap());
  TableHeap *table_heap = TableHeap::Create(buffer_pool_manager_, schema_, txn,
                                            log_manager_, lock_manager_, table_info->GetMemHeap());
  TableMetadata *table_meta = TableMetadata::Create(tableId, table_name,
                                                    table_heap->GetFirstPageId(), schema_, table_info->GetMemHeap());

  table_info->Init(table_meta, table_heap);
  // 为了持久化存储，新建一个页存储table_meta
  auto page = buffer_pool_manager_->NewPage(pageId);
  table_meta->SerializeTo(page->GetData());
  buffer_pool_manager_->UnpinPage(pageId, true); // 记得unpin
  // 初始化table_page的信息
//  page->Init(pageId, INVALID_PAGE_ID, log_manager_, txn); // 在table_heap中已经Init
  // 更新catalog_meta中的映射关系
  catalog_meta_->table_meta_pages_.insert({tableId, pageId});
  table_names_.insert({table_name, tableId});
  tables_.insert({tableId, table_info});
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTable(const string &table_name, TableInfo *&table_info) {
  // ASSERT(false, "Not Implemented yet");
  if(table_names_.find(table_name) == table_names_.end()){
    return DB_TABLE_NOT_EXIST;
  }
  table_info = tables_[table_names_[table_name]];
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTables(vector<TableInfo *> &tables) const {
  // ASSERT(false, "Not Implemented yet");
  for(auto pair : table_names_){
    tables.push_back(tables_.at(pair.second));
  }
  return DB_SUCCESS;
}

dberr_t CatalogManager::CreateIndex(const std::string &table_name, const string &index_name,
                                    const std::vector<std::string> &index_keys, Transaction *txn,
                                    IndexInfo *&index_info) {
  // ASSERT(false, "Not Implemented yet");
  if(table_names_.find(table_name) == table_names_.end()){
    return DB_TABLE_NOT_EXIST;
  }
  if(index_names_.find(table_name) != index_names_.end()){
    if(index_names_.at(table_name).find(index_name) != index_names_.at(table_name).end()){
      return DB_INDEX_ALREADY_EXIST;
    }
  }
  // 判断index_keys是否在schema中
  vector<uint32_t> key_map;
  table_id_t tableId = table_names_[table_name];
  TableInfo *tableInfo = tables_.at(tableId);
  auto schema = tableInfo->GetSchema();
  auto columns = schema->GetColumns();
  for(const auto& s : index_keys){
    int flag = 0;
    for(uint32_t i = 0; i < columns.size(); i++){
      if(columns[i]->GetName() == s){
        flag = 1;
        key_map.push_back(i);
        break;
      }
    }
    if(!flag){
      return DB_COLUMN_NAME_NOT_EXIST;
    }
  }
  // 创建index_info
  index_id_t indexId = catalog_meta_->GetNextIndexId();
  index_info = IndexInfo::Create(heap_);
  IndexMetadata *indexMetadata = IndexMetadata::Create(indexId, index_name, tableId, key_map,index_info->GetMemHeap());
  index_info->Init(indexMetadata, tableInfo, buffer_pool_manager_);
  // 将catalog_meta序列化存储到磁盘中
  page_id_t pageId;
  auto page = buffer_pool_manager_->NewPage(pageId);
  indexMetadata->SerializeTo(page->GetData());
  buffer_pool_manager_->UnpinPage(pageId, true);
  // 更新映射信息
  catalog_meta_->index_meta_pages_.insert({indexId, pageId});
  // 更新catalog
  if(index_names_.find(table_name) == index_names_.end()) {
    unordered_map<std::string, index_id_t> map;
    map.insert({index_name, indexId});
    index_names_.insert({table_name, map});
  }
  else{
    index_names_[table_name].insert({index_name, indexId});
  }
  indexes_.insert({indexId, index_info});
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetIndex(const std::string &table_name, const std::string &index_name,
                                 IndexInfo *&index_info) const {
  // ASSERT(false, "Not Implemented yet");
  if(index_names_.find(table_name) == index_names_.end()){
    return DB_TABLE_NOT_EXIST;
  }
  auto map = index_names_.at(table_name);
  if(map.find(index_name) == map.end()){
    return DB_INDEX_NOT_FOUND;
  }
  auto indexId = map[index_name];
  index_info = indexes_.at(indexId);
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTableIndexes(const std::string &table_name, std::vector<IndexInfo *> &indexes) const {
  // ASSERT(false, "Not Implemented yet");
  if(index_names_.find(table_name) == index_names_.end()){
    return DB_TABLE_NOT_EXIST;
  }
  auto map = index_names_.at(table_name);
  for(auto pair : map){
    indexes.push_back(indexes_.at(pair.second));
  }
  return DB_SUCCESS;
}

dberr_t CatalogManager::DropTable(const string &table_name) {
  // ASSERT(false, "Not Implemented yet");
  if(table_names_.find(table_name) == table_names_.end()){
    return DB_TABLE_NOT_EXIST;
  }
  table_id_t tableId = table_names_[table_name];
  //TableInfo *table_info = tables_[tableId];
  //table_info->GetTableHeap()->FreeHeap();
  page_id_t pageId = catalog_meta_->table_meta_pages_[tableId];
  buffer_pool_manager_->DeletePage(pageId);
  catalog_meta_->table_meta_pages_.erase(tableId);
  table_names_.erase(table_name);
  tables_.erase(tableId);
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}

dberr_t CatalogManager::DropIndex(const string &table_name, const string &index_name) {
  // ASSERT(false, "Not Implemented yet");
  if(index_names_.find(table_name) == index_names_.end()){
    return DB_TABLE_NOT_EXIST;
  }
  auto map = index_names_.at(table_name);
  if(map.find(index_name) == map.end()){
    return DB_INDEX_NOT_FOUND;
  }
  index_id_t indexId = map.at(index_name);
  IndexInfo *index_info = indexes_[indexId];
  index_info->GetIndex()->Destroy();
  page_id_t pageId = catalog_meta_->index_meta_pages_[indexId];
  buffer_pool_manager_->DeletePage(pageId);
  catalog_meta_->index_meta_pages_.erase(indexId);
  index_names_.erase(table_name);
  indexes_.erase(indexId);
  FlushCatalogMetaPage();
  return DB_SUCCESS;
}


dberr_t CatalogManager::FlushCatalogMetaPage() const {
  // ASSERT(false, "Not Implemented yet");
  auto page = buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  catalog_meta_->SerializeTo(page->GetData());
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID, true);
  return DB_SUCCESS;
}

dberr_t CatalogManager::LoadTable(const table_id_t table_id, const page_id_t page_id) {
  // ASSERT(false, "Not Implemented yet");
  auto page = buffer_pool_manager_->FetchPage(page_id);
  TableInfo *table_info = TableInfo::Create(heap_);
  TableMetadata *table_meta = nullptr;
  TableMetadata::DeserializeFrom(page->GetData(), table_meta, table_info->GetMemHeap());
  TableHeap *table_heap = TableHeap::Create(buffer_pool_manager_, table_meta->GetFirstPageId(),
                                            table_meta->GetSchema(),
                                            log_manager_, lock_manager_, table_info->GetMemHeap());
  table_info->Init(table_meta, table_heap);
  buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  string table_name = table_meta->GetTableName();
  // 更新映射信息
  table_names_.insert({table_name, table_id});
  tables_.insert({table_id, table_info});
  return DB_SUCCESS;
}

dberr_t CatalogManager::LoadIndex(const index_id_t index_id, const page_id_t page_id) {
  // ASSERT(false, "Not Implemented yet");
  auto page = buffer_pool_manager_->FetchPage(page_id);
  IndexInfo *index_info = IndexInfo::Create(heap_);
  IndexMetadata *index_meta = nullptr;
  IndexMetadata::DeserializeFrom(page->GetData(), index_meta, index_info->GetMemHeap());
  table_id_t tableId = index_meta->GetTableId();
  TableInfo *table_info;
  GetTable(tableId, table_info);
  index_info->Init(index_meta, table_info, buffer_pool_manager_);
  buffer_pool_manager_->UnpinPage(page_id, false);
  // 更新映射信息
  string table_name = table_info->GetTableName();
  string index_name = index_meta->GetIndexName();
  if(index_names_.find(table_name) == index_names_.end()){
    unordered_map<std::string, index_id_t> map;
    map.insert({index_name, index_id});
    index_names_.insert({table_name, map});
  }
  else{
    index_names_[table_name].insert({index_name, index_id});
  }
  indexes_.insert({index_id, index_info});
  return DB_SUCCESS;
}

dberr_t CatalogManager::GetTable(const table_id_t table_id, TableInfo *&table_info) {
  // ASSERT(false, "Not Implemented yet");
  table_info = tables_[table_id];
  return DB_SUCCESS;
}