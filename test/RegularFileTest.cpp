#include <gtest/gtest.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include "vfs/VFS.h"

auto filePath = "/home/maple/workspace/code/vfs/test/data/file2.txt";
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

void test_write(VFS::IFile::Buffer buf, std::size_t offset) {
    EXPECT_EQ( file.write(buf, buf.size()), buf.size() );
    EXPECT_EQ( file.write(buf, buf.size()), buf.size() );
    EXPECT_EQ( file.write(buf, buf.size()), buf.size() );
}

void test_read() {
    auto b = file.read(10);
    EXPECT_EQ( b.size(), 10 );
    std::cout << "read: " << b.data() << "\n";
}

TEST(RegularFileTest, MultiThread) {
    VFS::IFile::Buffer b1(10, 'C');
    VFS::IFile::Buffer b2(20, 'D');
    std::thread t1(test_write, std::ref(b1), 0);
    std::thread t2(test_write, std::ref(b2), 20);
    std::thread t3(test_read );
    std::thread t4(test_read );

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}
