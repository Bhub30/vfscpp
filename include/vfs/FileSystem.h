#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <mutex>
#include <string>
#include "IFS.h"
#include "IFile.h"
#include "global.h"

namespace VFS {

class FileSystem : public IFS
{
public:
    FileSystem(std::string const & path);
    DISABLE_COPY(FileSystem);
    ~FileSystem();

    std::string path() const override { return _path; };

    bool isMounted() const override { return _mounted; }

    bool mount(std::string const & path) override;

    bool unmount() override;

    IFilePtr open(std::string const & filename, Perms mode = Perms::RW) override;

    bool remove(std::string const & filename) override;

    bool touchFile(std::string const & filename) override;

    bool makeDir(std::string const & dir) override;

    bool moveTo(std::string const & from, std::string const & to) override;

    bool moveTo(std::string const & from, IFSPtr fsptr, std::string const & to) override;

    EntryList list() override;

    EntryList list(std::string const & dir) override;

    bool contain(std::string const & filename) override;

    std::string search(std::string const & filename) override;

    bool copy(std::string const & from, std::string const & to) override;

    type::FILETYPE type(std::string const & filename) override;

private:
    bool validFilename(std::string const & filename);

    bool hasPermision(Perms perm);

private:
    std::string _path;
    bool _mounted;
    std::mutex _mutex;
};

}

#endif // !FILESYSTEM_H
