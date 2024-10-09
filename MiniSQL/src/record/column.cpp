#include "record/column.h"

Column::Column(std::string column_name, TypeId type, uint32_t index, bool nullable, bool unique)
        : name_(std::move(column_name)), type_(type), table_ind_(index),
          nullable_(nullable), unique_(unique) {
  ASSERT(type != TypeId::kTypeChar, "Wrong constructor for CHAR type.");
  switch (type) {
    case TypeId::kTypeInt :
      len_ = sizeof(int32_t);
      break;
    case TypeId::kTypeFloat :
      len_ = sizeof(float_t);
      break;
    default:
      ASSERT(false, "Unsupported column type.");
  }
}

Column::Column(std::string column_name, TypeId type, uint32_t length, uint32_t index, bool nullable, bool unique)
        : name_(std::move(column_name)), type_(type), len_(length),
          table_ind_(index), nullable_(nullable), unique_(unique) {
  ASSERT(type == TypeId::kTypeChar, "Wrong constructor for non-VARCHAR type.");
}

Column::Column(const Column *other) : name_(other->name_), type_(other->type_), len_(other->len_),
                                      table_ind_(other->table_ind_), nullable_(other->nullable_),
                                      unique_(other->unique_) {}

uint32_t Column::SerializeTo(char *buf) const {
  // replace with your code here
  uint32_t len = 0;
  MACH_WRITE_UINT32(buf, COLUMN_MAGIC_NUM);
  len += sizeof(uint32_t);

  uint32_t size = name_.length();
  MACH_WRITE_UINT32(buf + len, size);
  len += sizeof(uint32_t);

  MACH_WRITE_STRING(buf + len, name_);
  len += name_.length();

  MACH_WRITE_TO(TypeId, buf + len, type_);
  len += sizeof(TypeId);

  MACH_WRITE_UINT32(buf + len, len_);
  len += sizeof(uint32_t);

  MACH_WRITE_UINT32(buf + len, table_ind_);
  len += sizeof(uint32_t);

  MACH_WRITE_TO(bool, buf + len, nullable_);
  len += sizeof(bool);

  MACH_WRITE_TO(bool, buf + len, unique_);
  len += sizeof(bool);
  return len;
}

uint32_t Column::GetSerializedSize() const {
  // replace with your code here
  uint32_t len = 0;
  len += 4*sizeof(uint32_t) + 2*sizeof(bool) + name_.length() + sizeof(TypeId);
  return len;
}

uint32_t Column::DeserializeFrom(char *buf, Column *&column, MemHeap *heap) {
  // replace with your code here
  uint32_t len = 0;
  uint32_t magic_num = MACH_READ_UINT32(buf);
  ASSERT(magic_num == COLUMN_MAGIC_NUM, "wrong column serialized");
  len += sizeof(uint32_t);

  uint32_t length = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  std::string name_;
  char name[length + 1];
  memset(name, 0, length + 1);
  memcpy(name, buf + len, length);
  for(uint32_t i = 0; i < length; i++){
    name_.push_back(name[i]);
  }
  len += length;

  TypeId typeId = MACH_READ_FROM(TypeId, buf + len);
  len += sizeof(TypeId);

  uint32_t l = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  uint32_t index = MACH_READ_UINT32(buf + len);
  len += sizeof(uint32_t);

  bool nullable = MACH_READ_FROM(bool, buf + len);
  len += sizeof(bool);

  bool unique = MACH_READ_FROM(bool, buf + len);
  len += sizeof(bool);

  void *mem = heap->Allocate(sizeof(Column));
  if(typeId == TypeId::kTypeChar)
    column = new(mem)Column(name, typeId, l, index, nullable, unique);
  else
    column = new(mem)Column(name, typeId, index, nullable, unique);

  return len;
}
