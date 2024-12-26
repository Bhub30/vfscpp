#ifndef FILEINFO_H
#define FILEINFO_H

#include <string>

namespace VFS {

struct FileInfo
{
    typedef char const * const PermisionsT;

    constexpr static PermisionsT READ = "r-";
    constexpr static PermisionsT WRITE = "-w";
    constexpr static PermisionsT RW = "rw";

    std::string _type;
    PermisionsT _permisions;
    std::size_t _size;
    std::string _modifiedTime;
    std::string _name;
};

} // namespace VFS

#endif // !FILEINFO_H
