#include <algorithm>
#include <filesystem>
#include <fstream>
#include "IFS.h"
#include "IFile.h"
#include "RegularFile.h"
#include "FileSystem.h"

namespace VFS {

namespace fs = std::filesystem;

FileSystem::FileSystem(std::string const & path)
    : _path (path)
      , _mounted(false)
{
    if (*_path.rbegin() != '/')
    {
        _path.push_back('/');
    }

    mount(_path);
}

FileSystem::~FileSystem() { unmount(); }

bool FileSystem::mount(std::string const & path)
{
    if ( _mounted )
        return true;

    if ( !fs::exists(path) )
    {
        _path = "";
        return false;
    }
    if ( !fs::is_directory(fs::status(path)))
    {
        _path = "";
        return false;
    }

    fs::current_path(_path);

    return true;
}

bool FileSystem::unmount()
{
    if ( !_mounted )
        return false;

    _mounted = false;
    _path = "";

    return true;
}

IFS::IFilePtr FileSystem::open(std::string const & filename, Perms mode)
{
    if ( !_mounted || !validFilename(filename) || !hasPermision(mode) )
        return nullptr;

    return !fs::is_directory(filename) ? IFilePtr( new RegularFile(_path + filename) ) : nullptr;
}

bool FileSystem::remove(std::string const & filename)
{
    if ( !_mounted )
        return false;

    if ( !validFilename(filename) || fs::exists(filename) )
        return false;

    return fs::remove(filename);
}

bool FileSystem::touchFile(std::string const & filename)
{
    if ( !_mounted || !validFilename(filename) )
        return false;
    if ( !fs::exists(filename) )
        return false;

    std::ofstream{filename};

    return true;
}

bool FileSystem::makeDir(std::string const & filename)
{
    if ( !_mounted )
        return false;

    if ( !validFilename(filename) || fs::exists(filename) )
        return false;

    return fs::create_directory(filename);
}

bool FileSystem::moveTo(std::string const & from, std::string const & to)
{
    if ( !_mounted || !validFilename(from) || !validFilename(to) )
        return false;
    if ( !fs::exists(from) || !fs::exists(to))
        return false;

    fs::rename(from, to);

    return true;

}

bool FileSystem::moveTo(std::string const from, IFSPtr & fs, std::string const & to)
{
    if ( !_mounted || fs == nullptr )
        return false;

    if ( !validFilename(from) || !validFilename(to) )
        return false;

    if ( !fs::exists(from) || !fs::exists(to))
        return false;

    fs::rename(from, to);

    return true;

}

IFS::EntryList FileSystem::list()
{
    if ( !_mounted )
        return {};

    return list(".");
}

IFS::EntryList FileSystem::list(std::string const & dir)
{
    if ( !_mounted || !validFilename(dir))
        return {};

    if ( !fs::exists(dir) )
        return {};

    auto iters = fs::recursive_directory_iterator(dir);
    EntryList result;
    result.reserve(10);
    for (auto const& dir_entry : iters)
    {
        result.push_back(std::move(dir_entry.path().string()));
    }

    return result;
}

bool FileSystem::contain(std::string const & filename)
{
    return search(filename) == type::NOTFOUND;
}

std::string FileSystem::search(std::string const & filename)
{
    if ( !_mounted || validFilename(filename) )
        return type::NOTFOUND;

    auto entry = list();
    auto result = std::find_if(entry.begin(), entry.end(), [&] (std::string const & item) -> bool
    {
        return item.find(filename) != std::string::npos;
    });

    return result != entry.end() ? *result : type::NOTFOUND;
}

bool FileSystem::copy(std::string const & from, std::string const & to)
{
    if ( !_mounted )
        return false;

    if ( !validFilename(from) || !validFilename(to) )
        return false;

    if ( !fs::exists(from) || !fs::exists(to))
        return false;

    return fs::copy_file(from, to);
}

type::FILETYPE FileSystem::type(std::string const & filename)
{
    if ( !_mounted || !validFilename(filename) )
        return type::NOTFOUND;

    fs::file_status fs{fs::status(filename)};
    switch (fs.type())
    {
        case fs::file_type::fifo:
            return type::PIPE;
            break;
        case fs::file_type::unknown:
            return type::UNKOWN;
            break;
        case fs::file_type::socket:
            return type::SOCKET;
            break;
        case fs::file_type::symlink:
            return type::SYMLINK;
            break;
        case fs::file_type::directory:
            return type::DIRECTORY;
            break;
        case fs::file_type::regular:
            return type::REGULAR;
            break;
        case fs::file_type::block:
            return type::BLOCK;
            break;
        case fs::file_type::not_found:
            return type::NOTFOUND;
            break;
        case fs::file_type::none:
            return type::NONE;
            break;
        default:
            return type::IMPLDEFINE;
            break;
    }
}

bool FileSystem::validFilename(std::string const & filename)
{
    auto it = filename.begin();
    std::uint8_t prefix_with_double_dot = 0;
    static auto fn = [&prefix_with_double_dot] (char ch)
    {
        if ( ch == '.' ) ++prefix_with_double_dot;
    };
    std::for_each(it, it + 1, fn);
    if ( !fs::exists(filename) || *filename.begin() == '/' || prefix_with_double_dot == 2  )
        return false;

    return true;
}

bool FileSystem::hasPermision(Perms perm)
{
    using std::filesystem::perms;

    auto const & allPerms = fs::status(_path).permissions();
    perms per = ( perm == Perms::READ ? perms::owner_read : perms::owner_write );

    return perms::none == ( perms::others_read & per ) ? false : true;
}

}
