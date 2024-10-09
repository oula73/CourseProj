#include "record/schema.h"

uint32_t Schema::SerializeTo(char *buf) const {
  // replace with your code here
  uint32_t len = 0;
  MACH_WRITE_UINT32(buf, SCHEMA_MAGIC_NUM);
  len += sizeof(uint32_t);

  uint32_t size = columns_.size();
  MACH_WRITE_UINT32(buf + len, size);
  len += sizeof(uint32_t);

  for(size_t i = 0; i < size; i++){
    len += columns_[i]->SerializeTo(buf + len);
  }

  return len;
}

uint32_t Schema::GetSerializedSize() const {
  // replace with your code here
  uint32_t len = 0;
  for(auto column : columns_){
    len += column->GetSerializedSize();
  }
  return len + 2 * sizeof(uint32_t);
}

uint32_t Schema::DeserializeFrom(char *buf, Schema *&schema, MemHeap *heap) {
  // replace with your code here
  uint32_t len = 0;
  uint32_t magic_num = MACH_READ_UINT32(buf);
  ASSERT(magic_num == SCHEMA_MAGIC_NUM, "wrong schema serialized");
  len += sizeof(uint32_t);

  uint32_t size = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  std::vector<Column*> columns;
  for(uint32_t i = 0; i < size; i++){
    Column* column;
    len += Column::DeserializeFrom(buf + len, column, heap);
    columns.push_back(column);
  }

  void* mem = heap->Allocate(sizeof(schema));
  schema = new(mem)Schema(columns);

  return len;
}