#include "catalog/indexes.h"

IndexMetadata *IndexMetadata::Create(const index_id_t index_id, const string &index_name,
                                     const table_id_t table_id, const vector<uint32_t> &key_map,
                                     MemHeap *heap) {
  void *buf = heap->Allocate(sizeof(IndexMetadata));
  return new(buf)IndexMetadata(index_id, index_name, table_id, key_map);
}

uint32_t IndexMetadata::SerializeTo(char *buf) const {
  uint32_t len = 0;
  MACH_WRITE_UINT32(buf, INDEX_METADATA_MAGIC_NUM);
  len += sizeof(uint32_t);

  MACH_WRITE_UINT32(buf + len, index_id_);
  len += sizeof(index_id_t);

  uint32_t length = index_name_.length();
  MACH_WRITE_UINT32(buf + len, length);
  len += sizeof(uint32_t);

  MACH_WRITE_STRING(buf + len, index_name_);
  len += length;

  MACH_WRITE_UINT32(buf + len, table_id_);
  len += sizeof(table_id_t);

  uint32_t size = key_map_.size();
  MACH_WRITE_UINT32(buf + len, size);
  len += sizeof(uint32_t);

  for(uint32_t i = 0; i < size; i++){
    MACH_WRITE_UINT32(buf + len, key_map_[i]);
    len += sizeof(uint32_t);
  }

  return len;
}

uint32_t IndexMetadata::GetSerializedSize() const {
  uint32_t size = key_map_.size();
  uint32_t len = index_name_.length();
  return (3 + size) * sizeof(uint32_t) + sizeof(index_id_t) + sizeof(table_id_t) + len;
}

uint32_t IndexMetadata::DeserializeFrom(char *buf, IndexMetadata *&index_meta, MemHeap *heap) {
  uint32_t len = 0;
  uint32_t magic = MACH_READ_UINT32(buf);
  len += sizeof(uint32_t);
  ASSERT(magic == INDEX_METADATA_MAGIC_NUM, "wrong indexes serialize");

  uint32_t indexId = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  uint32_t length = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  std::string index_name;
  char name[length + 1];
  memset(name, 0, length + 1);
  memcpy(name, buf + len, length);
  for(uint32_t i = 0; i < length; i++){
    index_name.push_back(name[i]);
  }
  len += length;

  uint32_t tableId = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  uint32_t size = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  std::vector<uint32_t> map;
  for(uint32_t i = 0; i < size; i++){
    map.push_back(MACH_READ_UINT32(buf + len));
    len += sizeof(uint32_t);
  }

  void* mem = heap->Allocate(sizeof(IndexMetadata));
  index_meta = new(mem) IndexMetadata(indexId, index_name, tableId, map);

  return len;
}