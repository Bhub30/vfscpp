#ifndef REGULARFILE_H
#define REGULARFILE_H

#include <fstream>
#include <filesystem>
#include <string>
#include "IFile.h"
#include "global.h"

namespace VFS {

namespace fs = std::filesystem;

class RegularFile : public IFile
{
public:
    RegularFile(std::string const & filename);
    ~RegularFile();
    DISABLE_COPY(RegularFile);

    std::size_t write(Buffer const & buf, std::size_t size) override;

    std::size_t write(Buffer const & buf, std::size_t offset, std::size_t size) override;

    Buffer read(std::size_t size) override;

    Buffer readAll() override;

    Buffer read(std::size_t offset, std::size_t size) override;

    void close() override;

    FileInfo info() const override;

    std::size_t size() override;

    std::string filename() const override;

    FileInfo::PermisionsT permision() const override;

    void setPermision(Perms perms) override;

    void disableWrite() override;

    void disableRead() override;

private:
    std::string _filename;  // absolute path
    std::ofstream _stream;
    std::ifstream _inStream;
    bool _access;
    std::size_t _size;
    fs::perms _perms;
};

}

#endif // !REGULARFILE_H
