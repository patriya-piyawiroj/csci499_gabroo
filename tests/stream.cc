#include "gtest/gtest.h"
#include <glog/logging.h>

#include "kvstore/store.h"
#include "warble/functions.h"
#include "kvstore/db.h"

// A mock DB containing KVStore for testing
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

// Test class containing instance of mock DB
class StreamTest : public ::testing::Test {
 protected:
  FakeDB db;
  void SetUp() override {
    db.put("_users_", "alice");
    db.put("_users_", "white-rabbit");
  }
};

// Tests for helpers functions Hashtag and FindHashtag
TEST_F(StreamTest, FindsAllHashtags) {
  // Is able to identify correct number of hashtags form string
  EXPECT_EQ(0, Hashtag(&db, "id1", "no hashtags"));
  EXPECT_EQ(2, Hashtag(&db, "id2", "two hashtags #hello #greeting"));
  EXPECT_EQ(1, Hashtag(&db, "id3", "different id repeat hashtag #hello"));

  // Is able to find hashtags from multiple warbles
  std::vector<std::string> expected;
  EXPECT_EQ(expected, FindHashtag(&db, "#greet")); // Substring shouldn't count
  expected.push_back("id2");  
  EXPECT_EQ(expected, FindHashtag(&db, "#greeting")); // Should find one
  expected.push_back("id3");
  EXPECT_EQ(expected, FindHashtag(&db, "#hello")); // Should find two
}

// Tests Stream functionality
TEST_F(StreamTest, Stream) {
  Any req, rep;

  // Sets up new warbles with hashtags
  WarbleRequest w_req;
  w_req.set_username("alice") ;
  w_req.set_text("alice #wonderland");
  req.PackFrom(w_req);
  EXPECT_TRUE(Warble(&db, req, &rep));
  w_req.set_username("white-rabbit");  
  w_req.set_text("white rabbit #wonderland");
  req.PackFrom(w_req);
  EXPECT_TRUE(Warble(&db, req, &rep));

  // Streams hashtags from warbles
  StreamRequest s_req;
  s_req.set_hashtag("#wonderland");
  req.PackFrom(s_req);
  EXPECT_TRUE(Stream(&db, req, &rep));
  StreamReply s_rep;
  rep.UnpackTo(&s_rep);
  EXPECT_EQ(s_rep.warbles_size(), 2);

  // Unable to find nonexistent hashtag
  s_req.set_hashtag("#iDontExist");
  req.PackFrom(s_req);
  EXPECT_FALSE(Stream(&db, req, &rep));
}

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleTest(&argc,argv);
  return RUN_ALL_TESTS();
}
