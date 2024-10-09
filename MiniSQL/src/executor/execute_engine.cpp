#include "executor/execute_engine.h"
#include "glog/logging.h"
extern "C" {
int yyparse(void);
#include "parser/minisql_lex.h"
#include "parser/parser.h"
}

ExecuteEngine::ExecuteEngine() {
//  string filename = "db_name.txt";
//  ifstream in(filename.c_str());
//  if(in){
//    string line;
//    while(getline(in, line)){
//      auto db = new DBStorageEngine(line, false);
//      dbs_.insert({line, db});
//    }
//  }
  current_db_ = "db2";
  auto db = new DBStorageEngine(current_db_, true);
  dbs_.insert({current_db_, db});
}

dberr_t ExecuteEngine::Execute(pSyntaxNode ast, ExecuteContext *context) {
  if (ast == nullptr) {
    return DB_FAILED;
  }
  switch (ast->type_) {
    case kNodeCreateDB:
      return ExecuteCreateDatabase(ast, context);
    case kNodeDropDB:
      return ExecuteDropDatabase(ast, context);
    case kNodeShowDB:
      return ExecuteShowDatabases(ast, context);
    case kNodeUseDB:
      return ExecuteUseDatabase(ast, context);
    case kNodeShowTables:
      return ExecuteShowTables(ast, context);
    case kNodeCreateTable:
      return ExecuteCreateTable(ast, context);
    case kNodeDropTable:
      return ExecuteDropTable(ast, context);
    case kNodeShowIndexes:
      return ExecuteShowIndexes(ast, context);
    case kNodeCreateIndex:
      return ExecuteCreateIndex(ast, context);
    case kNodeDropIndex:
      return ExecuteDropIndex(ast, context);
    case kNodeSelect:
      return ExecuteSelect(ast, context);
    case kNodeInsert:
      return ExecuteInsert(ast, context);
    case kNodeDelete:
      return ExecuteDelete(ast, context);
    case kNodeUpdate:
      return ExecuteUpdate(ast, context);
    case kNodeTrxBegin:
      return ExecuteTrxBegin(ast, context);
    case kNodeTrxCommit:
      return ExecuteTrxCommit(ast, context);
    case kNodeTrxRollback:
      return ExecuteTrxRollback(ast, context);
    case kNodeExecFile:
      return ExecuteExecfile(ast, context);
    case kNodeQuit:
      return ExecuteQuit(ast, context);
    default:
      break;
  }
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteCreateDatabase(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteCreateDatabase" << std::endl;
#endif
  pSyntaxNode child = ast->child_;
  string db_name = child->val_;
  auto *db = new DBStorageEngine(db_name, true);
  dbs_.insert({db_name, db});
  string filename = "db_name.txt";
  ofstream of(filename.c_str(), ofstream::app);
  of << db_name << endl;
  of.close();
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteDropDatabase(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDropDatabase" << std::endl;
#endif
  pSyntaxNode child = ast->child_;
  string db_name = child->val_;
  // TODO
  dbs_.erase(db_name);
  delete dbs_[db_name];
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteShowDatabases(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteShowDatabases" << std::endl;
#endif
  for(auto &pair : dbs_) {
    cout << pair.first << endl;
  }
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteUseDatabase(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteUseDatabase" << std::endl;
#endif
  pSyntaxNode child = ast->child_;
  current_db_ = child->val_;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteShowTables(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteShowTables" << std::endl;
#endif
  auto db = dbs_[current_db_];
  vector<TableInfo *> tables;
  db->catalog_mgr_->GetTables(tables);
  for(auto tableinfo : tables) {
   cout << tableinfo->GetTableName() << endl;
  }
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteCreateTable(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteCreateTable" << std::endl;
#endif
  auto db = dbs_[current_db_];
  pSyntaxNode node = ast->child_;
  string table_name = node->val_;
  vector<Column *> columns;
  node = node->next_;
  // 如果没有行
  if (node) {
    node = node->child_;
  }
  else{
    perror("cannot create a table without any column!");
    //exit(-1);
  }
  char u[] = "unique";
  char primary[] = "primary keys";
  int index = 0;
  while(node != nullptr) {
    bool unique = false;
    if(node->val_ != nullptr && strcmp(node->val_, primary) == 0){
      // TODO: 不懂primary应该放在哪里
    }
    else {
      if(node->val_ != nullptr && strcmp(node->val_, u) == 0){
        unique = true;
      }
      pSyntaxNode h = node->child_;
      TypeId typeId = kTypeInvalid;
      string column_name = h->val_;
      h = h->next_;
      string type = h->val_;
      if (type == "char") {
        typeId = TypeId::kTypeChar;
      } else if (type == "int") {
        typeId = TypeId::kTypeInt;
      } else if (type == "float") {
        typeId = TypeId::kTypeFloat;
      } else {
        perror("not implement");
      }
      Column *column;
      if (typeId == TypeId::kTypeChar) {
        int len = atoi(h->child_->val_);
        // TODO: 还没决定是否为null
        column = new Column(column_name, typeId, len, index++, false, unique);
      } else {
        column = new Column(column_name, typeId, index++, false, unique);
      }
      columns.push_back(column);
    }
    node = node->next_;
  }
  auto schema = std::make_shared<Schema>(columns);
  TableInfo *tableInfo;
  db->catalog_mgr_->CreateTable(table_name, schema.get(), nullptr, tableInfo);
  cout << "Successfully create a table" << endl;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteDropTable(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDropTable" << std::endl;
#endif
  auto db = dbs_[current_db_];
  pSyntaxNode node = ast->child_;
  string table_name = node->val_;
  db->catalog_mgr_->DropTable(table_name);
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteShowIndexes(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteShowIndexes" << std::endl;
#endif
  //auto db = new DBStorageEngine(current_db_, false);
  // TODO: 没有指明是哪个table？
  //db->catalog_mgr_->GetTableIndexes();
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteCreateIndex(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteCreateIndex" << std::endl;
#endif
  clock_t start, end;
  start = clock();
  auto db = dbs_[current_db_];
  pSyntaxNode node = ast->child_;
  string index_name = node->val_;
  node = node->next_;
  string table_name = node->val_;
  TableInfo *tableInfo;
  if (db->catalog_mgr_->GetTable(table_name, tableInfo) != DB_SUCCESS) {
    cout << "cannot find the table" << endl;
    return DB_TABLE_NOT_EXIST;
  }
  Schema *schema = tableInfo->GetSchema();
  node = node->next_->child_;
  vector<string> index_keys;
  vector<uint32_t> key_map;
  while(node != nullptr) {
    string index = node->val_;
    uint32_t index_;
    if(schema->GetColumnIndex(index, index_) != DB_SUCCESS){
      cout << "no such column" << endl;
      return DB_COLUMN_NAME_NOT_EXIST;
    }
    const Column *column = schema->GetColumn(index_);
    if(column->GetType() == TypeId::kTypeFloat){
      cout << "can only create index on unique index" << endl;
      return DB_FAILED;
    }
    index_keys.push_back(index);
    key_map.push_back(index_);
    node = node->next_;
  }
  IndexInfo *indexInfo;
  db->catalog_mgr_->CreateIndex(table_name, index_name, index_keys, nullptr, indexInfo);
  TableHeap *tableHeap = tableInfo->GetTableHeap();
  Index *bptree = indexInfo->GetIndex();
  for(auto it = tableHeap->Begin(nullptr); it != tableHeap->End(); ++it){
    Row row = *it;
    vector<Field*> fields = row.GetFields();
    vector<Field> key_field;
    for(auto i : key_map){
      key_field.push_back(*fields[i]);
    }
    Row key_row(key_field);
    bptree->InsertEntry(key_row, row.GetRowId(), nullptr);
  }
  end = clock();
  cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;
  cout << "Successfully create a index" << endl;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteDropIndex(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDropIndex" << std::endl;
#endif
  auto db = dbs_[current_db_];
  pSyntaxNode node = ast->child_;
  string index_name = node->val_;
  string table_name_;
  vector<TableInfo *> tables;
  db->catalog_mgr_->GetTables(tables);
  for(auto table : tables) {
    int flag = 0;
    string table_name = table->GetTableName();
    vector<IndexInfo *> indexes;
    db->catalog_mgr_->GetTableIndexes(table_name, indexes);
    for(auto i : indexes) {
      if(i->GetIndexName() == index_name){
        table_name_ = table_name;
        flag = 1;
        break;
      }
    }
    if(flag){
      break;
    }
  }
  db->catalog_mgr_->DropIndex(table_name_,index_name);
  return DB_SUCCESS;
}

bool compare(string comparator, Field *f, string value){
  bool compare_result = true;
  if(f->type_id_ == TypeId::kTypeChar){
    if(comparator == "=")
      compare_result = strcmp(f->value_.chars_, value.c_str()) == 0;
    else if(comparator == ">")
      compare_result = strcmp(f->value_.chars_, value.c_str()) > 0;
    else if(comparator == "<")
      compare_result = strcmp(f->value_.chars_, value.c_str()) < 0;
    else if(comparator == ">=")
      compare_result = strcmp(f->value_.chars_, value.c_str()) >= 0;
    else if(comparator == "<=")
      compare_result = strcmp(f->value_.chars_, value.c_str()) <= 0;
  }
  else if(f->type_id_ == TypeId::kTypeInt){
    if(comparator == "=")
      compare_result = f->value_.integer_ == atoi(value.c_str());
    else if(comparator == ">")
      compare_result = f->value_.integer_ > atoi(value.c_str());
    else if(comparator == "<")
      compare_result = f->value_.integer_ < atoi(value.c_str());
    else if(comparator == ">=")
      compare_result = f->value_.integer_ >= atoi(value.c_str());
    else if(comparator == "<=")
      compare_result = f->value_.integer_ <= atoi(value.c_str());
  }
  else{
    if(comparator == "=")
      compare_result = fabs(f->value_.float_ - atof(value.c_str())) <= 1E-6;
    else if(comparator == ">")
      compare_result = f->value_.float_ > atof(value.c_str());
    else if(comparator == "<")
      compare_result = f->value_.float_ < atof(value.c_str());
    else if(comparator == ">=")
      compare_result = f->value_.float_ >= atof(value.c_str());
    else if(comparator == "<=")
      compare_result = f->value_.float_ <= atof(value.c_str());
  }
  return compare_result;
}

bool printornot(vector<uint32_t> &v, uint32_t i){
  for(auto j : v){
    if(i == j)
      return true;
  }
  return false;
}

dberr_t ExecuteEngine::ExecuteSelect(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteSelect" << std::endl;
#endif
  clock_t start, end;
  start = clock();
  auto db = dbs_[current_db_];
  pSyntaxNode node = ast->child_;
  /// 如果需要投影
  char *val = node->val_;
  vector<uint32_t> projection;
  pSyntaxNode p = node->child_;

  node = node->next_;
  string table_name = node->val_;

  TableInfo *tableInfo;
  db->catalog_mgr_->GetTable(table_name, tableInfo);
  TableHeap *tableHeap = tableInfo->GetTableHeap();

  if(val != nullptr){
    while (p != nullptr){
      string name = p->val_;
      uint32_t i;
      tableInfo->GetSchema()->GetColumnIndex(name, i);
      projection.push_back(i);
      p = p->next_;
    }
  }
  else{
    uint32_t num = tableInfo->GetSchema()->GetColumnCount();
    for(uint32_t i = 0; i < num; i++){
      projection.push_back(i);
    }
  }

  string comparator, comparator1;
  string column_name, column_name1;
  string value, value1;
  char *v;
  uint32_t column_index, column_index1;
  string connector;
  if(node->next_ != nullptr){
    node = node->next_->child_;
    if(node->type_ == SyntaxNodeType::kNodeCompareOperator){
      comparator = node->val_;
      node = node->child_;
      column_name = node->val_;
      node = node->next_;
      v = node->val_;
      value = node->val_;
      tableInfo->GetSchema()->GetColumnIndex(column_name, column_index);
    }
    else{
      connector = node->val_;
      node = node->child_;
      pSyntaxNode h = node->next_;
      comparator = node->val_;
      node = node->child_;
      column_name = node->val_;
      node = node->next_;
      value = node->val_;
      tableInfo->GetSchema()->GetColumnIndex(column_name, column_index);

      comparator1 = h->val_;
      h = h->child_;
      column_name1 = h->val_;
      h = h->next_;
      value1 = h->val_;
      tableInfo->GetSchema()->GetColumnIndex(column_name1, column_index1);
    }
  }

  vector<IndexInfo *> indexes;
  db->catalog_mgr_->GetTableIndexes(table_name, indexes);
  if(!indexes.empty()){
    IndexInfo *indexInfo = indexes[0];
    uint32_t i;
    if(indexInfo->GetIndexKeySchema()->GetColumnIndex(column_name, i) != DB_COLUMN_NAME_NOT_EXIST){
      Index *index = indexInfo->GetIndex();
      const Column *c = indexInfo->GetIndexKeySchema()->GetColumn(i);
      uint32_t len = c->GetLength();
      Field field(TypeId::kTypeChar, v, len, true);
      vector<Field> fs;
      fs.push_back(field);
      Row r(fs);
      vector<RowId> res;
      index->ScanKey(r, res, nullptr);
      for(auto j : res){
        Row *row = new Row(j);
        tableHeap->GetTuple(row, nullptr);
        vector<Field*> hh = row->GetFields();
        for (uint32_t k = 0; k < hh.size(); k++) {
          if (printornot(projection, k)) {
            Field *ff = hh[k];
            //cout << "B+ tree result" << endl;
            if (ff->type_id_ == TypeId::kTypeChar) {
              cout << ff->value_.chars_;
            } else if (ff->type_id_ == TypeId::kTypeInt) {
              cout << ff->value_.integer_;
            } else if (ff->type_id_ == TypeId::kTypeFloat) {
              cout << ff->value_.float_;
            }
            cout << " ";
          }
        }
        cout << endl;
        cout << "B+ tree res" << endl;
      }
      end = clock();
      cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;
      return DB_SUCCESS;
    }
  }

  auto itr = tableHeap->Begin(nullptr);
  for(; itr!= tableHeap->End(); ++itr){
    Row row = *itr;
    vector<Field*> fields = row.GetFields();
    bool compare_result = true;
    if(!comparator.empty()){
      Field *f = fields[column_index];
      compare_result = compare(comparator, f, value);
      if(!connector.empty()){
        Field *f1 = fields[column_index1];
        bool c = compare(comparator1, f1, value1);
        if(connector == "and"){
          compare_result = compare_result && c;
        }
        else if(connector == "or")
          compare_result = compare_result || c;
      }
    }
    if(compare_result) {
      for (uint32_t i = 0; i < fields.size(); i++) {
        if (printornot(projection, i)) {
          Field *field = fields[i];
          if (field->type_id_ == TypeId::kTypeChar) {
            cout << field->value_.chars_;
          } else if (field->type_id_ == TypeId::kTypeInt) {
            cout << field->value_.integer_;
          } else if (field->type_id_ == TypeId::kTypeFloat) {
            cout << field->value_.float_;
          }
          cout << " ";
        }
      }
      cout << endl;
    }
  }
  end = clock();
  cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteInsert(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteInsert" << std::endl;
#endif
  auto db = dbs_[current_db_];
  pSyntaxNode node = ast->child_;
  string table_name = node->val_;
  node = node->next_->child_;
  TableInfo *tableInfo;
  db->catalog_mgr_->GetTable(table_name, tableInfo);
  TableHeap *tableHeap = tableInfo->GetTableHeap();
  Schema *schema = tableInfo->GetSchema();
  vector<Field> fields;
  int index = 0;
  while(node != nullptr) {
    char* val = node->val_;
    auto column = schema->GetColumn(index++);
    TypeId typeId = column->GetType();
    if(typeId == TypeId::kTypeChar){
      uint32_t len = column->GetLength();
      Field field = Field(typeId, val, len, true);
      fields.push_back(field);
    }
    else if(typeId == TypeId::kTypeFloat){
      float data = atof(val);
      Field field = Field(typeId, data);
      fields.push_back(field);
    }
    else{
      int data = atoi(val);
      Field field = Field(typeId, data);
      fields.push_back(field);
    }
    node = node->next_;
  }
  Row row(fields);
  /// 保证唯一约束，为了性能，用B+树索引
  vector<IndexInfo *> indexes;
  db->catalog_mgr_->GetTableIndexes(table_name, indexes);
  for(auto j : indexes){
    Index *idx = j->GetIndex();
    vector<Field> f;
    f.push_back(fields[1]);
    Row r(f);
    vector<RowId> res;
    idx->ScanKey(r, res, nullptr);
    if(!res.empty()){
      cout << "cannot insert duplicate name on account" << endl;
      return DB_FAILED;
    }
  }
  tableHeap->InsertTuple(row ,nullptr);
  //cout << "Query OK. 1 line has changed" << endl;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteDelete(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteDelete" << std::endl;
#endif
  auto db = dbs_[current_db_];
  pSyntaxNode node = ast->child_;
  string table_name = node->val_;
  TableInfo *tableInfo;
  db->catalog_mgr_->GetTable(table_name, tableInfo);
  TableHeap *tableHeap = tableInfo->GetTableHeap();
  if(node->next_ == nullptr){
    auto it = tableHeap->Begin(nullptr);
    for(;it != tableHeap->End(); it++){
      tableHeap->MarkDelete((*it).GetRowId(), nullptr);
    }
    return DB_SUCCESS;
  }
  node = node->next_->child_->child_;
  string column_name = node->val_;
  node = node->next_;
  char *val = node->val_;

  vector<IndexInfo *> indexes;
  db->catalog_mgr_->GetTableIndexes(table_name, indexes);
  if(!indexes.empty()){
    IndexInfo *indexInfo = indexes[0];
    uint32_t i;
    if(indexInfo->GetIndexKeySchema()->GetColumnIndex(column_name, i) != DB_COLUMN_NAME_NOT_EXIST){
      Index *index = indexInfo->GetIndex();
      const Column *c = indexInfo->GetIndexKeySchema()->GetColumn(i);
      uint32_t len = c->GetLength();
      Field field(TypeId::kTypeChar, val, len, true);
      vector<Field> fs;
      fs.push_back(field);
      Row r(fs);
      vector<RowId> res;
      index->ScanKey(r, res, nullptr);
      for(auto h : res){
        index->RemoveEntry(r, h, nullptr);
        tableInfo->GetTableHeap()->MarkDelete(h, nullptr);
      }
      return DB_SUCCESS;
    }
  }

  uint32_t index;
  tableInfo->GetSchema()->GetColumnIndex(column_name, index);
  TypeId typeId = tableInfo->GetSchema()->GetColumn(index)->GetType();
  auto itr = tableHeap->Begin(nullptr);
  for(; itr != tableHeap->End(); itr++){
    Row row = *itr;
    vector<Field*> fields = row.GetFields();
    for(auto field : fields){
      if(typeId == field->GetType()){
        if(typeId == TypeId::kTypeChar){
          if(strcmp(val, field->value_.chars_) == 0){
            tableHeap->MarkDelete(row.GetRowId(), nullptr);
          }
        }
        else if(typeId == TypeId::kTypeInt){
          if(atoi(val) == field->value_.integer_){
            tableHeap->MarkDelete(row.GetRowId(), nullptr);
          }
        }
        else if(typeId == TypeId::kTypeFloat){
          if(fabs(atof(val) - field->value_.float_) <= 1E-6){
            tableHeap->MarkDelete(row.GetRowId(), nullptr);
          }
        }
      }
    }
  }
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteUpdate(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteUpdate" << std::endl;
#endif

  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteTrxBegin(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteTrxBegin" << std::endl;
#endif
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteTrxCommit(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteTrxCommit" << std::endl;
#endif
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteTrxRollback(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteTrxRollback" << std::endl;
#endif
  return DB_FAILED;
}

dberr_t ExecuteEngine::ExecuteExecfile(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteExecfile" << std::endl;
#endif
  pSyntaxNode node = ast->child_;
  string filename = node->val_;
  //string file = "/mnt/d/minisql/minisql-master/src/executor/" + filename;
  ifstream in(filename.c_str(), ios::in);
  int cnt = 0;
  clock_t start, end;
  start = clock();
  if(in){
    string line;
    while(getline(in, line)) {
      YY_BUFFER_STATE bp = yy_scan_string(line.c_str());
      if (bp == nullptr) {
        LOG(ERROR) << "Failed to create yy buffer state." << std::endl;
        exit(1);
      }
      yy_switch_to_buffer(bp);

      // init parser module
      MinisqlParserInit();

      // parse
      yyparse();

      Execute(MinisqlGetParserRootNode(), context);
      //sleep(1);
      cnt++;

      // clean memory after parse
      MinisqlParserFinish();
      yy_delete_buffer(bp);
      yylex_destroy();
    }
  }
  end = clock();
  cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;
  cout << cnt << endl;
  return DB_SUCCESS;
}

dberr_t ExecuteEngine::ExecuteQuit(pSyntaxNode ast, ExecuteContext *context) {
#ifdef ENABLE_EXECUTE_DEBUG
  LOG(INFO) << "ExecuteQuit" << std::endl;
#endif
  ASSERT(ast->type_ == kNodeQuit, "Unexpected node type.");
  context->flag_quit_ = true;
  return DB_SUCCESS;
}
