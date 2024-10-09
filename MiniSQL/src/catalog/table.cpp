#include "catalog/table.h"

uint32_t TableMetadata::SerializeTo(char *buf) const {
  uint32_t len = 0;
  MACH_WRITE_UINT32(buf, TABLE_METADATA_MAGIC_NUM);
  len += sizeof(uint32_t);

  MACH_WRITE_UINT32(buf + len, table_id_);
  len += sizeof(table_id_t);

  uint32_t length = table_name_.length();
  MACH_WRITE_UINT32(buf + len, length);
  len += sizeof(uint32_t);

  MACH_WRITE_STRING(buf + len, table_name_);
  len += length;

  MACH_WRITE_INT32(buf + len, root_page_id_);
  len += sizeof(page_id_t);

  /// 这里一开始没明白，写错了，应该是schema的序列化与反序列化，这样才能存储到磁盘中
//  auto addr = reinterpret_cast<uint64_t>(schema_);
//  MACH_WRITE_TO(uint64_t , buf + len, addr);
//  len += sizeof(uint64_t);
  schema_->SerializeTo(buf + len);
  len += schema_->GetSerializedSize();

  return len;
}

uint32_t TableMetadata::GetSerializedSize() const {
  uint32_t length = table_name_.length();
  return length + 3 * sizeof(uint32_t) + sizeof(page_id_t) + schema_->GetSerializedSize();
}

/**
 * @param heap Memory heap passed by TableInfo
 */
uint32_t TableMetadata::DeserializeFrom(char *buf, TableMetadata *&table_meta, MemHeap *heap) {
  uint32_t len = 0;
  uint32_t magic = MACH_READ_UINT32(buf);
  len += sizeof(uint32_t);
  ASSERT(magic == TABLE_METADATA_MAGIC_NUM, "wrong table serialize");

  uint32_t tableId = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  uint32_t length = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  std::string table_name;
  char name[length + 1];
  memset(name, 0, length + 1);
  memcpy(name, buf + len, length);
  for(uint32_t i = 0; i < length; i++){
    table_name.push_back(name[i]);
  }
  len += length;

  page_id_t pageId = MACH_READ_INT32(buf + len);
  len += sizeof(uint32_t);

  /// 应该是schema的反序列化
//  uint64_t addr = MACH_READ_FROM(uint64_t ,buf + len);
//  Schema* schema = reinterpret_cast<Schema*>(addr);
//  len += sizeof(uint64_t);
  Schema *schema;
  Schema::DeserializeFrom(buf + len, schema, heap);

  void* mem = heap->Allocate(sizeof(TableMetadata));
  table_meta = new(mem) TableMetadata(tableId, table_name, pageId, schema);

  return len;
}

/**
 * Only called by create table
 *
 * @param heap Memory heap passed by TableInfo
 */
TableMetadata *TableMetadata::Create(table_id_t table_id, std::string table_name,
                                     page_id_t root_page_id, TableSchema *schema, MemHeap *heap) {
  // allocate space for table metadata
  void *buf = heap->Allocate(sizeof(TableMetadata));
  return new(buf)TableMetadata(table_id, table_name, root_page_id, schema);
}

TableMetadata::TableMetadata(table_id_t table_id, std::string table_name, page_id_t root_page_id, TableSchema *schema)
        : table_id_(table_id), table_name_(table_name), root_page_id_(root_page_id), schema_(schema) {}
