#include "gtest/gtest.h"
#include <glog/logging.h>

#include "kvstore/store.h"
#include "warble/functions.h"
#include "kvstore/db.h"

class FakeDB : public Database {
 public:
  bool put(std::string key, std::string val) { return store_.put(key, val); }
  std::optional<std::vector<std::string>> get(std::string key) {
    return store_.get(key);
  }
  bool remove(std::string key) { return store_.remove(key); }
 private:
  KVStore store_;
};

class StreamTest : public ::testing::Test {
 protected:
  FakeDB db;
};


TEST_F(StreamTest, FindsAllHashtags) {
  EXPECT_EQ(0, Hashtag(&db, "id1", "no hashtags"));
  EXPECT_EQ(2, Hashtag(&db, "id2", "two hashtags #hello #greeting"));
  EXPECT_EQ(1, Hashtag(&db, "id3", "different id repeat hashtag #hello"));

  std::vector<std::string> expected;
  expected.push_back("id2");  
  EXPECT_EQ(expected, FindHashtag(&db, "#greeting"));

  expected.push_back("id3");
  EXPECT_EQ(expected, FindHashtag(&db, "#hello")); 
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
