/*
  Sajeev Saluja
  @gabroo
  kvstore/store.h

  Backend key value store headers. See kvstore/store.cc for implementation.
*/

#ifndef KVSTORE_H
#define KVSTORE_H

#include <glog/logging.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "protos/kvstore.pb.h"

// Backend data structure for storing values in memory, using
// std::unordered_map.
class KVStore {
 public:
  bool put(const std::string&, const std::string&);
  // Keys that do not have values will return std::nullopt from get
  std::optional<std::vector<std::string>> get(const std::string&);
  bool remove(const std::string&);
  // Dumps the contents of the kvstore to the file
  void dump(const std::string&);
  // Reads the contents of the file to the kvstore
  void read(const std::string&);

 private:
  std::unordered_map<std::string, std::vector<std::string>> map_;
  std::shared_mutex mu_;
};

#endif  // !KVSTORE_H