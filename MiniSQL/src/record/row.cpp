#include "record/row.h"

uint32_t Row::SerializeTo(char *buf, Schema *schema) const {
  // replace with your code here
  uint32_t len = 0;
  MACH_WRITE_INT32(buf, rid_.GetPageId());
  len += sizeof(page_id_t);

  MACH_WRITE_UINT32(buf + len, rid_.GetSlotNum());
  len += sizeof(uint32_t);

  uint32_t size = fields_.size();
  MACH_WRITE_UINT32(buf + len, size);
  len += sizeof(uint32_t);

  char null_bitmap[size/8 + 1];
  memset(null_bitmap, 0, size/8 + 1);
  for(size_t i = 0; i < size; i++){
    if(fields_[i]->IsNull()){
      uint32_t id = i / 8;
      uint8_t offset = i % 8;
      null_bitmap[id] = null_bitmap[id] | (1 << offset);
    }
  }

  uint32_t bitmap_size = size / 8 + 1;
  MACH_WRITE_UINT32(buf + len, bitmap_size);
  len += sizeof(uint32_t);

  memcpy(buf + len, null_bitmap, bitmap_size);
  len += bitmap_size;

  for(auto field : fields_){
    TypeId typeId = field->GetType();
    MACH_WRITE_TO(TypeId, buf + len, typeId);
    len += sizeof(TypeId);

    len += field->SerializeTo(buf + len);
  }

  return len;
}

uint32_t Row::DeserializeFrom(char *buf, Schema *schema) {
  // replace with your code here
  uint32_t len = 0;
  //page_id_t pageId = MACH_READ_UINT32(buf);
  len += sizeof(page_id_t);

  //uint32_t slot_num = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  //rid_.Set(pageId, slot_num);

  uint32_t size = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

//  void *mem = heap_->Allocate(sizeof(RowId));
//  RowId *rid = new(mem)RowId(pageId, slot_num);

  uint32_t bitmap_size = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  char null_bitmap[size/8 + 1];
  memset(null_bitmap, 0, size/8 + 1);
  memcpy(null_bitmap, buf + len, bitmap_size);
  len += bitmap_size;

  fields_.clear();

  for(size_t i = 0; i < size; i++){
    TypeId typeId = MACH_READ_FROM(TypeId, buf + len);
    len += sizeof(TypeId);

    uint32_t id = i / 8;
    uint8_t offset = i % 8;
    std::bitset<8> bt(null_bitmap[id]);
    auto* field = ALLOC_P(heap_, Field)(typeId);
    len += Field::DeserializeFrom(buf + len, typeId, &field, bt.test(offset), heap_);
    fields_.push_back(field);
  }

  return len;
}

uint32_t Row::GetSerializedSize(Schema *schema) const {
  // replace with your code here
  // 似乎有问题，cy
  uint32_t len = 0;
  for(auto field : fields_){
    len += sizeof(TypeId);
    len += field->GetSerializedSize();
  }
  uint32_t size = GetFieldCount();
  return sizeof(page_id_t) + 3*sizeof(uint32_t) + len + size / 8 + 1;
}
