/*
  Sajeev Saluja
  @gabroo
  kvstore/store.cc

  Basic key value store with the following API:
    `put(key, value)` associates `value` with `key` in the store
    `get(key)` returns stream of values associated with `key`
    `remove(key)` removes all values associated with `key`
*/

#include "store.h"

bool KVStore::put(const std::string &key, const std::string &value) {
  std::unique_lock<std::shared_mutex> lck(mu_);
  if (map_.find(key) == map_.end()) {
    map_[key].push_back(value);
  } else {
    std::vector<std::string> vals = map_[key];
    auto find = std::find(vals.begin(), vals.end(), value);
    if (find != vals.end()) {
      return false;
    } else {
      map_[key].push_back(value);
    }
  }
  return true;
}

std::optional<std::vector<std::string>> KVStore::get(const std::string &key) {
  std::shared_lock<std::shared_mutex> lck(mu_);
  if (map_.find(key) != map_.end()) {
    return map_[key];
  } else {
    return std::nullopt;
  }
}

bool KVStore::remove(const std::string &key) {
  std::unique_lock<std::shared_mutex> lck(mu_);
  if (map_.find(key) == map_.end()) {
    return false;  // could not find key in map
  } else {
    map_.erase(key);
    return true;
  }
}

void KVStore::read(const std::string &file) {
  LOG(INFO) << "reading store contents from\t" << file;
  std::ifstream fs(file);
  if (fs.is_open()) {
    // parse file into protobuf object
    kvstore::Store store;
    if (store.ParseFromIstream(&fs)) {
      std::unique_lock<std::shared_mutex> lck(mu_);
      std::string key;
      // populate map
      for (kvstore::Pair p : store.pairs()) {
        std::vector<std::string> values;
        key = p.key();
        for (std::string v : p.values()) values.push_back(v);
        map_[key] = values;
      }
    } else {
      LOG(ERROR) << "file exists, but can't parse contents";
    }
  } else {
    LOG(INFO) << "file not found, starting with clean kvstore";
  }
}

void KVStore::dump(const std::string &file) {
  std::shared_lock<std::shared_mutex> lck(mu_);
  kvstore::Store store;
  // populate store structure
  std::unordered_map<std::string, std::vector<std::string>>::iterator itr;
  for (itr = map_.begin(); itr != map_.end(); ++itr) {
    std::string key = itr->first;
    std::vector<std::string> vals = itr->second;
    kvstore::Pair kvpair;
    kvpair.set_key(key);
    for (std::string s : vals) *(kvpair.add_values()) = s;
    *(store.add_pairs()) = kvpair;
  }
  // write store to file
  LOG(INFO) << "dumping store contents to\t" << file;
  std::ofstream fs(file, std::ofstream::trunc);
  store.SerializeToOstream(&fs);
  fs.close();
}
