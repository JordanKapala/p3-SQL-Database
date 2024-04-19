// Project Identifier: C0F4DFE8B340D81183C208F70F9D2D797908754D
#include "TableEntry.h"

#include <getopt.h>
#include <functional>
#include <cassert>
#include <iostream>
#include <utility>
#include <exception>
#include <map>

using namespace std;

class Table {
  private:
  string name;
  bool quiet_mode;

  vector<EntryType> col_types; //needed? right implementation?
  vector<string>    col_names; //needed? right implementation?

  unordered_map<string, int> column_names; //test column, index (in col_types)
  
  vector<vector<TableEntry>> rows; //rows

  bool hashed = false;
  bool mapped = false;
  int generate_col_idx;
  unordered_map<TableEntry, vector<size_t>> hash; //Entry, Rows (ALL ROWS)
  map<TableEntry, vector<size_t>> bst;


  class EqualComp {
    private:
    string &col;
    TableEntry &value;
    unordered_map<string, int> &column_names;

    public:

    EqualComp(string & col, TableEntry & val, unordered_map<string, int> & col_names) 
            : col(col), value(val), column_names(col_names) {}

    bool operator() (const vector<TableEntry> & row) const {
      if (row[column_names[col]] == value) {
        return true;
      }
      return false;
    }
  };

  class LessComp {
    private:
    string &col;
    TableEntry &value;
    unordered_map<string, int> &column_names;

    public:

    LessComp(string & col, TableEntry & val, unordered_map<string, int> & col_names) 
            : col(col), value(val), column_names(col_names) {}

    bool operator() (const vector<TableEntry> & row) const {
      if (row[column_names[col]] < value) {
        return true;
      }
      return false;
    }
  };

  class GreaterComp {
    private:
    string &col;
    TableEntry &value;
    unordered_map<string, int> &column_names;

    public:

    GreaterComp(string & col, TableEntry & val, unordered_map<string, int> & col_names) 
            : col(col), value(val), column_names(col_names) {}

    bool operator() (const vector<TableEntry> & row) const {
      if (row[column_names[col]] > value) {
        return true;
      }
      return false;
    }
  };

  void DeleteHelper(TableEntry value, string & col_name, string & OP) {
    switch (OP[0]) {
    case '=':
      rows.erase(remove_if(rows.begin(), rows.end(), EqualComp(col_name, value, column_names)), rows.end());
      break;
    case '<':
      rows.erase(remove_if(rows.begin(), rows.end(), LessComp(col_name, value, column_names)), rows.end());
      break;
    case '>':
      rows.erase(remove_if(rows.begin(), rows.end(), GreaterComp(col_name, value, column_names)), rows.end());
      break;
    }
    // if (OP == "=") {
    //   rows.erase(remove_if(rows.begin(), rows.end(), EqualComp(col_name, value, column_names)), rows.end());
    // }
    // else if (OP == "<") {
    //   rows.erase(remove_if(rows.begin(), rows.end(), LessComp(col_name, value, column_names)), rows.end());
    // }
    // else if (OP == ">") {
    //   rows.erase(remove_if(rows.begin(), rows.end(), GreaterComp(col_name, value, column_names)), rows.end());
    // }
  }

  void Regenerate() {
    if (hashed) {
      hash.clear();
      size_t N = rows.size();
      hash.reserve(N);
      for (size_t i = 0; i < N; ++i) {
        auto it = hash.find(rows[i][generate_col_idx]);
        if (it != hash.end()) {
          it->second.emplace_back(i);
        }
        else {
          vector<size_t> value = {i};
          hash.emplace(rows[i][generate_col_idx], value);
        }
      }
    }
    else if (mapped) {
      bst.clear();
      size_t N = rows.size();
      for (size_t i = 0; i < N; ++i) {
        auto it = bst.find(rows[i][generate_col_idx]);
        if (it != bst.end()) {
          it->second.emplace_back(i);
        }
        else {
          vector<size_t> value = {i};
          bst.emplace(rows[i][generate_col_idx], value);
        }
      }
    }
  }

  void PrintWhereHelper(string & col_name, string & OP, TableEntry val, vector<size_t> & cols, size_t M, size_t N) {

    if (hashed && generate_col_idx == column_names[col_name] && OP == "=") { //use index
      auto it = hash.find(val);
      // auto it_upper = it;
      if (it == hash.end()) {
        cout << "Printed 0 matching rows from " << name << "\n";
        return ;
      }

      size_t printable_rows = it->second.size();
      if (!quiet_mode) {
      for (size_t i = 0; i < printable_rows; ++i) {
        for (size_t j = 0; j < N; ++j) {
        //   if (col_types[idx] != EntryType::Bool) {
            cout << rows[it->second[i]][cols[j]] << " ";
        //   }
        //   else {
        //     cout << (rows[it->second[i]][idx] == TableEntry(bool{true}) ? "true" : "false") << " ";
        //   }
        } 
        cout << "\n";
      }
      }
      cout << "Printed " << printable_rows << " matching rows from " << name << "\n";
    }
    else if (mapped && generate_col_idx == column_names[col_name]) {
      size_t rows_printed = 0;
      auto it = bst.end();
      auto it_upper = it;

      if (OP == "=") {
        it = bst.find(val);
        if (it == bst.end()) {
          cout << "Printed 0 matching rows from " << name << "\n";
          return ;
        }
        it_upper = it;
        it_upper++;
      }
      else if (OP == "<") {
        it = bst.begin();
        it_upper = bst.lower_bound(val);
        if (it_upper == it) {
          cout << "Printed 0 matching rows from " << name << "\n";
          return ;
        }
      }
      else if (OP == ">") {
        it = bst.upper_bound(val);
        it_upper = bst.end();
        if (it == it_upper) {
          cout << "Printed 0 matching rows from " << name << "\n";
          return ;
        }
      }

      while (it != it_upper) {
        size_t printable_rows = it->second.size();
        rows_printed += printable_rows;
        if (!quiet_mode) {
        for (size_t i = 0; i < printable_rows; ++i) {
          for (size_t j = 0; j < N; ++j) {
            // if (col_types[idx] != EntryType::Bool) {
              cout << rows[it->second[i]][cols[j]] << " ";
            // }
            // else {
            //   cout << (rows[it->second[i]][idx] == TableEntry(bool{true}) ? "true" : "false") << " ";
            // }
          } 
          cout << "\n";
        }
        }
        it++;
      }
      cout << "Printed " << rows_printed << " matching rows from " << name << "\n";

      // size_t printable_rows = it->second.size();
      // for (size_t i = 0; i < printable_rows; ++i) {
      //   for (size_t j = 0; j < N; ++j) {
      //     idx = cols[j];
      //     if (col_types[idx] != EntryType::Bool) {
      //       cout << rows[it->second[i]][idx] << " ";
      //     }
      //     else {
      //       cout << (rows[it->second[i]][idx] == TableEntry(bool{true}) ? "true" : "false") << " ";
      //     }
      //   } 
      //   cout << "\n";
      // }
    }
      else { //normal
      EqualComp   EQComp(col_name, val, column_names);
      LessComp    LTComp(col_name, val, column_names);
      GreaterComp GTComp(col_name, val, column_names);
        size_t rows_printed = 0;
        for (size_t i = 0; i < M; ++i) {
          if ((OP == "=" && EQComp(rows[i])) || 
              (OP == "<" && LTComp(rows[i])) || 
              (OP == ">" && GTComp(rows[i])) ){
            rows_printed++;
            if (!quiet_mode) {
            for (size_t j = 0; j < N; ++j) {
            //   if (col_types[idx] != EntryType::Bool) {
                cout << rows[i][cols[j]] << " ";
            //   }
            //   else {
            //     cout << (rows[i][idx] == TableEntry(bool{true}) ? "true" : "false") << " ";
            //   }
            }
            cout << "\n";
            }
          }
        }
        cout << "Printed " << rows_printed << " matching rows from " << name << "\n";
      }
  }
  public:

  //CREATE
  Table(string& name, int N, bool q) : name{name}, quiet_mode{q} {
    string item;
    // EntryType entry;
    
    col_types.reserve(N);
    col_names.reserve(N);
    column_names.reserve(N);

    // rows.reserve(N);

    for (int i = 0; i < N; ++i) {
      cin >> item;

      switch (item[0]) {
        case 's':
          col_types.emplace_back(EntryType::String);
          break;

        case 'd':
          col_types.emplace_back(EntryType::Double);
          break;

        case 'i':
          col_types.emplace_back(EntryType::Int);
          break;

        case 'b':
          col_types.emplace_back(EntryType::Bool);
          break;
      }
    }

    for (int i = 0; i < N; ++i) {
      cin >> item;
      cout << item << " ";
      col_names.push_back(item);
      column_names.emplace(item,i);
    }
  }

  void INSERT(size_t N) { //N is number of inserted rows
    size_t K = rows.size();
    size_t M = col_types.size();
    string data;

    rows.reserve(K+N);
    // for (size_t i = 0; i < M; ++i) {
    //   rows.reserve(K+N);
    // }
    if (hashed) {
      hash.reserve(hash.size() + N);
    }

    vector<TableEntry> new_row;
    new_row.reserve(M);
    for (size_t i = 0; i < N; ++i) { //row
      new_row.clear();
      for (size_t j = 0; j < M; ++j) { //col
        cin >> data;

        switch (col_types[j]) {
          case EntryType::String:
            new_row.emplace_back(TableEntry(data));
            break;
          
          case EntryType::Double:
            new_row.emplace_back(TableEntry(stod(data)));
            break;
          
          case EntryType::Int:
            new_row.emplace_back(TableEntry(stoi(data)));
            break;
          
          case EntryType::Bool:
            new_row.emplace_back(TableEntry(data[0] == 't' ? bool{true} : bool{false}));
            break;
        }
      }
      rows.emplace_back(new_row);

      if (hashed) {
        auto it = hash.find(new_row[generate_col_idx]);
        if (it != hash.end()) {
          it->second.emplace_back(K + i);
        }
        else {
          vector<size_t> value = {K + i};
          hash.emplace(new_row[generate_col_idx], value);
        }
      }
      else if (mapped) {
        auto it = bst.find(new_row[generate_col_idx]);
        if (it != bst.end()) {
          it->second.emplace_back(K+i);
        }
        else {
          vector<size_t> value = {K + i};
          bst.emplace(new_row[generate_col_idx], value);
        }
      }
    }

    cout << "Added " << N << " rows to " << name 
         << " from position " << K << " to " << K + N - 1 << "\n";

  }

  void PRINT(size_t N) {
    string col_name; //each col to print, and conditional col if WHERE
    string condition;
    vector<size_t> cols; //index of columns, as passed in (so maybe not in order)
    size_t M = rows.size();
    auto it = column_names.end();

    for (size_t i = 0; i < N; ++i) {
      cin >> col_name;
      it = column_names.find(col_name);
      if (it != column_names.end()) {
        cols.push_back(it->second);
      }
      else {
        cout << "Error during PRINT: " << col_name << 
                " does not name a column in " << name << "\n";
        getline(cin, col_name);
        return ; //not sure
      }
    }

    cin >> condition;

    if (condition == "ALL") {
      if (!quiet_mode) {

        for (size_t i = 0; i < N; ++i) {
          cout << col_names[cols[i]] << " ";
        }
        cout << "\n";

        for (size_t i = 0; i < M; ++i) {
          for (size_t j = 0; j < N; ++j) {
          //   if (col_types[idx] != EntryType::Bool) {
              cout << rows[i][cols[j]] << " ";
          //   }
          //   else {
          //     cout << (rows[i][idx] == TableEntry(bool{true}) ? "true" : "false") << " ";
          //   }
          }
          cout << "\n";
        }
      }
      cout << "Printed " << M << " matching rows from " << name << "\n";
    }
    else { //WHERE, next cin is <colname> <OP> <value>
      string OP;
      string value;

      cin >> col_name >> OP >> value;

      if (column_names.find(col_name) == column_names.end()) {
        cout << "Error during PRINT: " << col_name << 
                " does not name a column in " << name << "\n";
        getline(cin, col_name);
        return ; //not sure
      }

      if (!quiet_mode) {
      for (size_t i = 0; i < N; ++i) {
        cout << col_names[cols[i]] << " ";
      }
      cout << "\n";
      }

      switch (col_types[column_names[col_name]]) {
        case EntryType::String:
          PrintWhereHelper(col_name, OP, TableEntry(value), cols, M, N);
          break;
        
        case EntryType::Double:
          PrintWhereHelper(col_name, OP, TableEntry(stod(value)), cols, M, N);
          break;

        case EntryType::Int:
          PrintWhereHelper(col_name, OP, TableEntry(stoi(value)), cols, M, N);
          break;

        case EntryType::Bool:
          PrintWhereHelper(col_name, OP, TableEntry(value[0] == 't' ? bool{true} : bool{false}), cols, M, N);
          break;
      }
    }
  }

  void DELETE() {
    string col_name;
    string OP;
    string value;
    size_t old_size = rows.size();

    cin >> col_name;
    cin >> OP;
    cin >> value;

  auto it = column_names.find(col_name);
  if (it == column_names.end()) {
    cout << "Error during DELETE: " << col_name << 
         " does not name a column in " << name << "\n";
    getline(cin, value);
    return ;
  }
  else { //column exists in 2D vector
    EntryType type = col_types[it->second];
    switch (type) {
      case EntryType::String:
        DeleteHelper(TableEntry(value), col_name, OP);
        break;
      
      case EntryType::Double:
        DeleteHelper(TableEntry(stod(value)), col_name, OP);
        break;

      case EntryType::Int:
        DeleteHelper(TableEntry(stoi(value)), col_name, OP);
        break;
      
      case EntryType::Bool:
        DeleteHelper(TableEntry(value[0] == 't' ? bool{true} : bool{false}), col_name, OP);
        break;
    }

    // if (type == EntryType::String) {
    //   DeleteHelper(TableEntry(value), col_name, OP);
    // }
    // else if (type == EntryType::Double) {
    //   DeleteHelper(TableEntry(stod(value)), col_name, OP);
    // }
    // else if (type == EntryType::Int) {
    //   DeleteHelper(TableEntry(stoi(value)), col_name, OP);
    // }
    // else if (type == EntryType::Bool) {
    //   DeleteHelper(TableEntry(value == "true" ? bool{true} : bool{false}), col_name, OP);
    // }

  }

  if (hashed || mapped) {
    Regenerate();
  }

  cout << "Deleted " << old_size - rows.size() << " rows from " << name << "\n";
}

  void JOIN(Table &table2) {
    string col_name1;
    string col_name2;
    vector<string> print_cols;
    vector<int>    print_nums;
    string print_col;
    int print_num;
    size_t N;
    auto end = column_names.end();
    
    cin >> col_name1;
    cin >> print_col;
    cin >> col_name2;
    cin >> print_col >> print_col;
    cin >> N;

    auto it = column_names.find(col_name1);
    if (it == end) {
      cout << "Error during JOIN: " << col_name1 << 
         " does not name a column in " << name << "\n";
      getline(cin, col_name1);
      return ;
    }

    unordered_map<string, int> &column_names2 = table2.GetColumns();
    auto end2 = column_names2.end();

    auto it2 = column_names2.find(col_name2);
    if (it2 == end2) {
      cout << "Error during JOIN: " << col_name2 << 
         " does not name a column in " << table2.GetName() << "\n";
      getline(cin, col_name2);
      return ;
    }

    for (size_t i = 0; i < N; ++i) {
      cin >> print_col;
      cin >> print_num;

      if (print_num == 1) {
        if (column_names.find(print_col) == end) {
          getline(cin, col_name1);
            cout << "Error during JOIN: " << print_col << 
         " does not name a column in " << name << "\n";
         return ;
        }
      }
      else {
        if (column_names2.find(print_col) == end2) {
          getline(cin, col_name2);
            cout << "Error during JOIN: " << print_col << 
         " does not name a column in " << table2.GetName() << "\n";
         return ;
        }
      }
      print_cols.emplace_back(print_col);
      print_nums.emplace_back(print_num);
    }

    if (!quiet_mode) {
      for (size_t i = 0; i < N; ++i) {
        cout << print_cols[i] << " ";
      }
      cout << "\n";
    }

    unordered_map<TableEntry, vector<size_t>> hash2;
    table2.JOIN_HASH(col_name2, hash2);
    vector<vector<TableEntry>> & rows2 = table2.GetRows();
    //vector<EntryType> &col_types2 = table2.GetColTypes();


    size_t table1_size = rows.size();
    int col_idx = column_names[col_name1];

    size_t k;
    size_t rows_printed = 0;
    size_t second;

    for (size_t i = 0; i < table1_size; ++i) {
      TableEntry &current_val = rows[i][col_idx];
      auto it = hash2.find(current_val);
      if (it != hash2.end()) {
        second = it->second.size();
        if (!quiet_mode) {
        for (k = 0; k < second; ++k) {
            for (size_t j = 0; j < N; ++j) {
              string & current_col = print_cols[j];
              if (print_nums[j] == 1) {
                // if (col_types[column_names[current_col]] != EntryType::Bool) {
                  cout << rows[i][column_names[current_col]] << " ";
                //}
                // else {	
                //   cout << (rows[i][column_names[current_col]] == TableEntry(bool{true}) ?
                //       "true" : "false") << " ";
                // }
              }
              else {
                // if (col_types2[column_names2[current_col]] != EntryType::Bool) {
                  cout << rows2[it->second[k]][column_names2[current_col]] << " ";
                //}
                // else {
                //   cout << (rows2[it->second[k]][column_names2[current_col]] == TableEntry(bool{true}) ?
                //       "true" : "false") << " ";
                // }
              }
            }
            cout << "\n";
          }
        }
        rows_printed += second;
      }
    }

    cout << "Printed " << rows_printed << " rows from joining " <<
          name << " to " << table2.GetName() << "\n";
  hash2.clear();
  }

  vector<vector<TableEntry>> & GetRows() {
    return rows;
  }

  unordered_map<string, int> & GetColumns() {
    return column_names;
  }

  vector<EntryType> & GetColTypes () {
    return col_types;
  }

  string & GetName() {
    return name;
  }

  void GENERATE(string & idx_type, string & col_name) {
    auto it = column_names.find(col_name);
    if (it == column_names.end()) {
      cout << "Error during GENERATE: " << col_name << 
         " does not name a column in " << name << "\n";
      getline(cin, col_name);
      return ;
    }

    if (hashed) {
      hash.clear();
    }
    else if (mapped) {
      bst.clear();
    }

    if (idx_type == "hash") {
      hashed = true;
      mapped = false;
      generate_col_idx = it->second;
      size_t N = rows.size();
      hash.reserve(N);
      for (size_t i = 0; i < N; ++i) {
        auto it = hash.find(rows[i][generate_col_idx]);
        if (it != hash.end()) {
          it->second.emplace_back(i);
        }
        else {
          vector<size_t> value = {i};
          hash.emplace(rows[i][generate_col_idx], value);
        }
      }

      cout << "Created " << idx_type << " index for table " << name << " on column " 
           << col_name << ", with " << hash.size() << " distinct keys\n";
    }
    else {
      mapped = true;
      hashed = false;
      generate_col_idx = it->second;
      size_t N = rows.size();
      for (size_t i = 0; i < N; ++i) {
        auto it = bst.find(rows[i][generate_col_idx]);
        if (it != bst.end()) {
          it->second.emplace_back(i);
        }
        else {
          vector<size_t> value = {i};
          bst.emplace(rows[i][generate_col_idx], value);
        }
      }

      cout << "Created " << idx_type << " index for table " << name << " on column " 
           << col_name << ", with " << bst.size() << " distinct keys\n";
    }

  }

  void JOIN_HASH(string & col_name, unordered_map<TableEntry, vector<size_t>> & temp_hash) {
    auto it1 = column_names.find(col_name);
    if (it1 == column_names.end()) {
      cout << "Error during JOIN: " << col_name << 
         " does not name a column in " << name << "\n";
      getline(cin, col_name);
      return ;
    }

      int temp_generate_col_idx = it1->second;
      size_t N = rows.size();
      temp_hash.reserve(N);
      for (size_t i = 0; i < N; ++i) {
        auto it = temp_hash.find(rows[i][temp_generate_col_idx]);
        if (it != temp_hash.end()) {
          it->second.emplace_back(i);
        }
        else {
          vector<size_t> value = {i};
          temp_hash.emplace(rows[i][temp_generate_col_idx], value);
        }
    }
  }


  void clear() {
    rows.clear();
  }
};

class Database {
  private:
  bool quiet_mode = false;
  string cmd = "";
  unordered_map<string, Table> TableMap;
  string buffer;
  int N;

  void printHelp() {
        cout << "You are a miner armed with dynamite and stuck in a collapsed mine.\n";
        cout << "You will move tile by tile until to escape, blowing up rubble and TNT if necessary.\n";
        cout << "To run the game, you must pass in one or more mode types, as well as a input text file (mine matrix). \n";
        cout << "Allowed modes:\n";
        cout << "verbose (-v) - prints rubble clearings and TNT explosions, and where they take place \n";
        cout << "median  (-m) - prints the median rubble cleared at each clearing\n";
        cout << "stats   (-s) - at the end of the game, prints statistics on first, last, easiest, and hardest tiles cleared.\n";
    }


  public:

   void GetOptions(int argc, char *argv[])
    {
        int choice;
        int index = 0;
        option long_options[] = {
            {"quiet"  , no_argument, nullptr, 'q'},
            {"help"   , no_argument, nullptr, 'h'},
            {nullptr, 0, nullptr, '\0'},
        }; // long_options[]

        while ((choice = getopt_long(argc, argv, "hq", long_options, &index)) != -1)
        {
            switch (choice)
            {
            case 'h':
                printHelp();
                exit(0);

            case 'q':
            {
                quiet_mode = true;
                break;
            }

            default:
                cerr << "Error: Unknown command line option" << endl;
                exit(1);
            } // switch ..choice
        }
    }


  void Run() {

    string tablename;
    string idx_type;
    string col_name;
    auto end = TableMap.end();
    auto it = end;
    auto it2 = end;
    while (cmd[0] != 'Q') {
      cout << "% ";
      cin >> cmd;

      switch (cmd[0]) {

        case 'Q':
          getline(cin, buffer);
          //delete data
          // for (auto t : TableMap) {
          //   t.second.clear(); 
          // }
          cout << "Thanks for being silly!\n";
          break;
        
        case '#': 
          getline(cin, buffer);
          break;
      
        case 'C':
          cin >> tablename;
          cin >> N;
          it = TableMap.find(tablename);
          if (it == end) {
            cout << "New table " << tablename << " with column(s) ";
            TableMap.emplace(tablename, Table(tablename, N, quiet_mode));
            cout << "created\n";
          }
          else {
            cout << "Error during CREATE: Cannot create already existing table " << tablename << "\n";
            getline(cin, buffer);
          }
          break;

        case 'R':
          cin >> tablename;
          it = TableMap.find(tablename);
          if (it != end) {
            TableMap.erase(tablename);
            cout << "Table " << tablename << " removed\n";
          }
          else {
            cout << "Error during REMOVE: "<< tablename << " does not name a table in the database\n";
            getline(cin, buffer);
          }
          break;

        case 'I':
          cin >> buffer;
          cin >> tablename;
          cin >> N;
          cin >> buffer;
          it = TableMap.find(tablename);
          if (it != end) {
            it->second.INSERT(N);
          }
          else {
            cout << "Error during INSERT: "<< tablename << " does not name a table in the database\n";
            getline(cin, buffer);
          }
          break;

        case 'P':
          cin >> buffer;
          cin >> tablename;
          cin >> N;
          it = TableMap.find(tablename);
          if (it != end) {
            it->second.PRINT(N);
          }
          else {
            cout << "Error during PRINT: "<< tablename << " does not name a table in the database\n";
            getline(cin, buffer);
          }
          break;
          
        case 'D':
          cin >> buffer;
          cin >> tablename;
          cin >> buffer;
          it = TableMap.find(tablename);
          if (it != end) {
            it->second.DELETE();
          }
          else {
            cout << "Error during DELETE: "<< tablename << " does not name a table in the database\n";
            getline(cin, buffer);
          }
          break;
          
        case 'J':
          cin >> tablename;
          cin >> buffer;
          it = TableMap.find(tablename);
          if (it == end) {
            cout << "Error during JOIN: "<< tablename << " does not name a table in the database\n";
            getline(cin, buffer);
          }
          cin >> tablename;
          cin >> buffer;
          it2 = TableMap.find(tablename);
          if (it2 == end) {
            cout << "Error during JOIN: "<< tablename << " does not name a table in the database\n";
            getline(cin, buffer);
          }
          it->second.JOIN(it2->second);
          break;
        
        case 'G':
          cin >> buffer;
          cin >> tablename;
          cin >> idx_type;
          cin >> buffer >> buffer;
          it = TableMap.find(tablename);
          if (it != end) {
            cin >> col_name;
            it->second.GENERATE(idx_type, col_name);
          }
          else {
            cout << "Error during GENERATE: "<< tablename << " does not name a table in the database\n";
            getline(cin, buffer);
          }
          break;
        
        default:
          getline(cin, buffer);
          cout << "Error: unrecognized command \n";
          break;
      }
    }
  }
};


int main(int argc, char * argv[]) {
  ios_base::sync_with_stdio(false); // you should already have this
  cin >> std::boolalpha;  // add these two lines
  cout << std::boolalpha;
  Database object = Database();
  object.GetOptions(argc, argv);
  object.Run();

}

      // else if (cmd[0] == '#') {
      //   getline(cin, buffer);
      // }
      // else if (cmd == "CREATE") {
      //   cin >> tablename;
      //   cin >> N;
      //   auto it = TableMap.find(tablename);
      //   if (it == TableMap.end()) {
      //     cout << "New table " << tablename << " with column(s) ";
      //     TableMap.emplace(tablename, Table(tablename, N, quiet_mode));
      //     cout << "created\n";
      //   }
      //   else {
      //     cout << "Error during CREATE: Cannot create already existing table " << tablename << "\n";
      //     getline(cin, buffer);
      //   }
      // }
      // else if (cmd == "REMOVE") {
      //   cin >> tablename;
      //   auto it = TableMap.find(tablename);
      //   if (it != TableMap.end()) {
      //     TableMap.erase(tablename);
      //     cout << "Table " << tablename << " removed\n";
      //   }
      //   else {
      //     cout << "Error during REMOVE: "<< tablename << " does not name a table in the database\n";
      //     getline(cin, buffer);
      //   }
      // }
      // else if (cmd == "INSERT") {
      //   cin >> buffer;
      //   cin >> tablename;
      //   cin >> N;
      //   cin >> buffer;
      //   auto it = TableMap.find(tablename);
      //   if (it != TableMap.end()) {
      //     it->second.INSERT(N);
      //   }
      //   else {
      //     cout << "Error during INSERT: "<< tablename << " does not name a table in the database\n";
      //     getline(cin, buffer);
      //   }
      // }
      // else if (cmd == "PRINT" ) {
      //   cin >> buffer;
      //   cin >> tablename;
      //   cin >> N;
      //   auto it = TableMap.find(tablename);
      //   if (it != TableMap.end()) {
      //     it->second.PRINT(N);
      //   }
      //   else {
      //     cout << "Error during PRINT: "<< tablename << " does not name a table in the database\n";
      //     getline(cin, buffer);
      //   }
      // }
      // else if (cmd == "DELETE") {
      //   cin >> buffer;
      //   cin >> tablename;
      //   cin >> buffer;
      //   auto it = TableMap.find(tablename);
      //   if (it != TableMap.end()) {
      //     it->second.DELETE();
      //   }
      //   else {
      //     cout << "Error during DELETE: "<< tablename << " does not name a table in the database\n";
      //     getline(cin, buffer);
      //   }
      // }
      // else if (cmd == "JOIN") {
      //   cin >> tablename;
      //   cin >> buffer;
      //   auto it1 = TableMap.find(tablename);
      //   if (it1 == TableMap.end()) {
      //     cout << "Error during JOIN: "<< tablename << " does not name a table in the database\n";
      //     getline(cin, buffer);
      //   }
      //   cin >> tablename;
      //   cin >> buffer;
      //   auto it2 = TableMap.find(tablename);
      //   if (it2 == TableMap.end()) {
      //     cout << "Error during JOIN: "<< tablename << " does not name a table in the database\n";
      //     getline(cin, buffer);
      //   }
      //   it1->second.JOIN(it2->second);

      // }
      // else if (cmd == "GENERATE") {
      //   cin >> buffer;
      //   cin >> tablename;
      //   cin >> idx_type;
      //   cin >> buffer >> buffer;
      //   auto it = TableMap.find(tablename);
      //   if (it != TableMap.end()) {
      //     cin >> col_name;
      //     it->second.GENERATE(idx_type, col_name);
      //   }
      //   else {
      //     cout << "Error during GENERATE: "<< tablename << " does not name a table in the database\n";
      //     getline(cin, buffer);
      //   }