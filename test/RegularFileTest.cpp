#include <gtest/gtest.h>
#include "vfs/IFile.h"
#include "vfs/VFS.h"

auto filePath = "/home/maple/workspace/code/vfs/test/data/file.txt";
VFS::RegularFile file(filePath);

VFS::IFile::Buffer buf(100, 'A');
VFS::IFile::Buffer buf2(10, 'B');

TEST(RegularFileTest, Write) {
    EXPECT_EQ( file.write(buf, buf.size()), buf.size() );
    EXPECT_EQ( file.write(buf2, 10, buf2.size()), buf2.size() );
    EXPECT_EQ( file.write(buf, buf.size()), buf.size() );
}

TEST(RegularFileTest, Read) {
    auto data = file.read(10, 10);
    std::string str1{ data.begin(), data.end() };
    std::string str2{ buf2.begin(), buf2.end() };
    EXPECT_EQ( str1, str2 );
}

TEST(RegularFileTest, Size) {
    EXPECT_EQ( file.size(), VFS::fs::file_size(filePath) );
}

TEST(RegularFileTest, Permissions) {
    file.disableWrite();
    // EXPECT_EQ( file.readAll().size(), 0 );
    EXPECT_EQ( file.write(buf, buf.size()), -1 );
    EXPECT_TRUE( file.readAll() != VFS::IFile::Buffer{}  );
    file.disableRead();
    EXPECT_TRUE( file.readAll() == VFS::IFile::Buffer{}  );

    file.setPermision(VFS::Perms::RW);
    file.disableAll();
    EXPECT_EQ( file.write(buf, buf.size()), -1 );
    EXPECT_TRUE( file.readAll() == VFS::IFile::Buffer{}  );
    file.setPermision(VFS::Perms::RW);
}
