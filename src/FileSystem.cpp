#include <algorithm>
#include <mutex>
#include "vfs/RegularFile.h"
#include "vfs/FileSystem.h"

namespace VFS {

namespace fs = std::filesystem;

FileSystem::FileSystem(std::string const & path)
    : _path (path)
      , _mounted(false)
      , _mutex()
{
    if ( *_path.rbegin() != '/' )
        _path.push_back('/');

    mount(_path);
}

FileSystem::~FileSystem() { unmount(); }

bool FileSystem::mount(std::string const & path)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( _mounted )
        return false;

    if ( !fs::exists(path) || !fs::is_directory(path) )
    {
        _path = "";
        return false;
    }

    _path = path;
    _mounted = true;
    // fs::current_path(_path);

    return _mounted;
}

bool FileSystem::unmount()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted )
        return false;

    _mounted = false;
    _path = "";

    return true;
}

IFS::IFilePtr FileSystem::open(std::string const & filename, Perms mode)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted || !fs::exists(filename) || !validFilename(filename) || !hasPermision(mode) )
        return nullptr;

    auto absolute = _path + filename;
    return !fs::is_directory(absolute) ? IFilePtr( new RegularFile(absolute) ) : nullptr;
}

bool FileSystem::remove(std::string const & filename)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted )
        return false;

    auto absolute = _path + filename;
    if ( !validFilename(filename) || !fs::exists(absolute) )
        return false;

    return fs::remove(absolute);
}

bool FileSystem::touchFile(std::string const & filename)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted || !validFilename(filename) )
        return false;
    auto absolute = _path + filename;
    if ( fs::exists(absolute) )
        return false;

    std::ofstream{absolute};

    return true;
}

bool FileSystem::makeDir(std::string const & filename)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted )
        return false;

    auto absolute = _path + filename;
    if ( !validFilename(filename) || fs::exists(absolute) )
        return false;

    return fs::create_directory(absolute);
}

bool FileSystem::moveTo(std::string const & from, std::string const & to)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted || !validFilename(from) || !validFilename(to) )
        return false;
    auto fromAbsolute = _path + from;
    auto toAbsolute = _path + to;
    if ( !fs::exists(fromAbsolute) || fs::exists(toAbsolute))
        return false;

    fs::rename(fromAbsolute, toAbsolute);

    return true;
}

bool FileSystem::moveTo(std::string const & from, IFSPtr fsptr, std::string const & to)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted || fsptr == nullptr || !fsptr->isMounted() )
        return false;

    if ( !validFilename(from) || !validFilename(to) )
        return false;

    auto fromAbsolute = _path + from;
    auto toAbsolute = fsptr->path() + to;
    if ( !fs::exists(fromAbsolute) || fs::exists(toAbsolute))
        return false;

    fs::rename(fromAbsolute, toAbsolute);

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
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted || !validFilename(dir))
        return {};

    auto absolute = _path + dir;
    if ( !fs::exists(absolute) )
        return {};

    auto iters = fs::recursive_directory_iterator(absolute);
    EntryList result;
    result.reserve(10);
    for ( auto const& dir_entry : iters )
        result.emplace_back( std::move( dir_entry.path().string() ) );

    return result;
}

bool FileSystem::contain(std::string const & filename)
{
    return search(filename) != type::NOTFOUND;
}

std::string FileSystem::search(std::string const & filename)
{
    if ( !_mounted || !validFilename(filename) )
        return type::NOTFOUND;

    auto entry = list();
    auto it = std::find_if(entry.begin(), entry.end(), [&filename] (std::string const & item) -> bool
    {
        return item.find(filename) != std::string::npos;
    });

    return it != entry.end() ? *it : type::NOTFOUND;
}

bool FileSystem::copy(std::string const & from, std::string const & to)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if ( !_mounted )
        return false;

    if ( !validFilename(from) || !validFilename(to) )
        return false;

    auto fromAbsolute = _path + from;
    auto toAbsolute = _path + to;
    if ( !fs::exists(fromAbsolute) || fs::exists(toAbsolute))
        return false;

    return fs::copy_file(fromAbsolute, toAbsolute);
}

type::FILETYPE FileSystem::type(std::string const & filename)
{
    if ( std::lock_guard<std::mutex> lock(_mutex); !_mounted || !validFilename(filename) )
        return type::NOTFOUND;

    fs::file_status fs{fs::status(_path + filename)};
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
    auto fn = [&prefix_with_double_dot] (char ch)
    {
        if ( ch == '.' ) ++prefix_with_double_dot;
    };
    std::for_each(it, it + 2, fn);
    if ( *filename.begin() == '/' || prefix_with_double_dot == 2  )
        return false;

    return true;
}

bool FileSystem::hasPermision(Perms perm)
{
    if ( perm == Perms::RW )
        return true;

    using std::filesystem::perms;

    auto const & allPerms = fs::status(_path).permissions();
    perms per = ( perm == Perms::READ ? perms::owner_read : perms::owner_write );

    return perms::none != ( perms::owner_read & per ) && perms::none != ( perms::owner_write & per );
}

}
