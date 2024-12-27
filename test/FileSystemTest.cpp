#include <gtest/gtest.h>
#include "vfs/VFS.h"

auto fsDir =  "/home/maple/workspace/code/vfs/test/data/";
VFS::FileSystem fs( fsDir );

TEST(FileSystemTest, Mount) {
  EXPECT_EQ( fs.path(), fsDir );
  EXPECT_TRUE( !fs.mount("dir") );
  EXPECT_TRUE( fs.isMounted() );
  EXPECT_TRUE( fs.unmount() );
  EXPECT_TRUE( fs.mount(fsDir) );
}

TEST(FileSystemTest, MakeDir) {
  EXPECT_TRUE( fs.makeDir("./dir1") );
  EXPECT_TRUE( fs.makeDir("./dir1/sub1") );
  EXPECT_TRUE( !fs.makeDir("../dir2") );
  EXPECT_TRUE( !fs.makeDir("/dir2") );
}

TEST(FileSystemTest, TouchFile) {
  EXPECT_TRUE( fs.touchFile("./file1.txt") );
  EXPECT_TRUE( fs.touchFile("file2.txt") );
  EXPECT_TRUE( fs.touchFile("./dir1/file3.txt") );
  EXPECT_TRUE( fs.touchFile("./dir1/sub1/file4.txt") );
  EXPECT_TRUE( !fs.touchFile("../file5.txt") );
  EXPECT_TRUE( !fs.touchFile("/file6.txt") );
}

TEST(FileSystemTest, OpenFile) {
  EXPECT_TRUE( fs.open("file1.txt") != nullptr );
  EXPECT_TRUE( fs.open("./file2.txt") != nullptr );
  EXPECT_TRUE( fs.open("./dir1/file3.txt") != nullptr );
  EXPECT_TRUE( fs.open("./dir1/sub1/file4.txt") != nullptr );
  EXPECT_EQ( fs.open("../file1.txt"), nullptr );
  EXPECT_EQ( fs.open("/file1.txt"), nullptr );
  EXPECT_EQ( fs.open("./dir1/file.txt"), nullptr );
  EXPECT_EQ( fs.open("./dir1/sub1/file.txt"), nullptr );
}

TEST(FileSystemTest, Remove) {
  EXPECT_TRUE( fs.remove("file1.txt") );
  EXPECT_TRUE( fs.remove("./file2.txt") );
  EXPECT_TRUE( !fs.remove("/dir1/sub1/file4.txt") );
  EXPECT_TRUE( !fs.remove("../dir1/sub1/file4.txt") );
  EXPECT_TRUE( !fs.remove("./dir1/file.txt") );
  EXPECT_TRUE( fs.remove("./dir1/file3.txt") );
  EXPECT_TRUE( !fs.remove("./dir1/sub1/file.txt") );
  EXPECT_TRUE( fs.remove("./dir1/sub1/file4.txt") );
}

TEST(FileSystemTest, Move) {
  fs.touchFile("file1.txt");
  fs.touchFile("dir1/file2.txt");
  fs.touchFile("./dir1/sub1/file3.txt");

  EXPECT_TRUE( !fs.moveTo("file1.txt", "/dir1") );
  EXPECT_TRUE( !fs.moveTo("file1.txt", "../dir1") );
  EXPECT_TRUE( fs.moveTo("file1.txt", "file.txt") );
  EXPECT_TRUE( fs.moveTo("dir1/file2.txt", "file2.txt") );

  EXPECT_TRUE( !fs.moveTo("/dir1/sub1", "dir2") );
  EXPECT_TRUE( !fs.moveTo("../dir1/sub1", "dir2") );
  EXPECT_TRUE( fs.moveTo("dir1/sub1", "dir2") );
  EXPECT_TRUE( !fs.moveTo("dir1/sub1/file3.txt", "file3.txt") );

  // auto dir =  "/home/maple/workspace/code/vfs/test/data2/";
  // VFS::IFS::IFSPtr fs2 = std::make_shared<VFS::FileSystem>(dir);
  // EXPECT_TRUE( fs2->isMounted() );
  // EXPECT_TRUE( !fs2->makeDir("dir") );
  // EXPECT_TRUE( fs.moveTo("file.txt", fs2, "dir/file.txt") );
  //
  // EXPECT_TRUE( !fs.remove("./*") );
  // EXPECT_TRUE( !fs2->remove("./dir/*") );
}

TEST(FileSystemTest, ListEntry) {
  auto && entry = fs.list();
  EXPECT_TRUE( entry.size() != 0 );
  // std::for_each(entry.begin(), entry.end(), [&] (std::string & str) { std::cout << str << "\n"; });
  entry = fs.list("/");
  EXPECT_EQ( entry.size(), 0 );
}

TEST(FileSystemTest, Search) {
  fs.touchFile("dir1/file2.txt");
  fs.makeDir("dir1/sub1");
  EXPECT_TRUE( fs.contain("file.txt") );
  EXPECT_TRUE( fs.contain("file2.txt") );
  EXPECT_TRUE( fs.contain("./dir1/sub1") );
  EXPECT_TRUE( fs.contain("sub1") );
  EXPECT_EQ( fs.search("file2.txt"), "./dir1/file2.txt" );
}

TEST(FileSystemTest, Copy) {
  EXPECT_TRUE( fs.copy( "file.txt", "file_copy.txt" ) );
}
