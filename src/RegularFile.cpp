#include "vfs/RegularFile.h"
#include "vfs/IFS.h"
#include "vfs/IFile.h"

namespace VFS {

namespace fs = std::filesystem;

RegularFile::RegularFile(std::string const & filename)
    : _filename(filename)
      , _file(_filename, std::ios::in | std::ios::out | std::ios::app)
      , _access(false)
      , _perms()
      , _writing(false)
      , _mutex()
      , _cv()
{
    if ( fs::exists(_filename) && _file.is_open() )
    {
        _access = true;
        _perms = fs::status(_filename).permissions();
    }
}

RegularFile::~RegularFile()
{
    close();
}


std::size_t RegularFile::write(IFile::Buffer const & buf, std::size_t size)
{
    return write(buf, 0, size);
}

std::size_t RegularFile::write(Buffer const & buf, std::size_t offset, std::size_t size)
{
    std::unique_lock<std::mutex> lk(_mutex);
    if ( !_access || ( _perms & fs::perms::owner_write ) == fs::perms::none )
        return 0;

    _writing = true;
    _cv.wait(lk, [this] () { return _writing; });

    if ( offset > 0 )
        _file.seekp(offset, std::ios::beg);
    auto beginPos = _file.tellp();
    auto getPos = _file.tellg();
    _file.write(buf.data(), size);
    if ( _file.fail() )
        return 0;
    auto endPos = _file.tellp();
    _file.seekg(getPos, std::ios::beg);
    if ( offset > 0 )
        _file.seekp(0, std::ios::end);

    _writing = false;
    lk.unlock();
    _cv.notify_one();

    return endPos - beginPos;
}

RegularFile::Buffer RegularFile::read(std::size_t size)
{
    return read(0, size);
}

RegularFile::Buffer RegularFile::readAll()
{
    std::size_t totalSize = 0;
    {
        std::lock_guard<std::mutex> lk(_mutex);
        totalSize = size();
    }

    return read(0, totalSize);
}

RegularFile::Buffer RegularFile::read(std::size_t offset, std::size_t size)
{
    std::unique_lock<std::mutex> lk(_mutex);
    if ( !_access || ( _perms & fs::perms::owner_read ) == fs::perms::none )
        return {};

    auto totalSize= this->size();
    if ( offset > totalSize )
        return {};

    auto validSize = ( size < totalSize ? size : totalSize );

    _cv.wait(lk, [this] () { return !_writing; });

    if ( offset > 0 )
        _file.seekg(offset, std::ios::beg);
    Buffer buf(validSize, '.');
    _file.read(buf.data(), validSize);

    lk.unlock();

    if ( _file.fail() )
        return {};

    return buf;
}

void RegularFile::close()
{
    std::lock_guard<std::mutex> lk(_mutex);
    if ( !_access )
        return;

    _file.flush();
    _file.close();

    _access = false;
}

FileInfo RegularFile::info() const
{
    using namespace std::chrono;

    auto fileTime = fs::last_write_time(_filename);
    auto sctp = time_point_cast<system_clock::duration>(fileTime - fs::file_time_type::clock::now() + system_clock::now());
    auto cftime = system_clock::to_time_t(sctp);

    return { type::REGULAR, permision(), size(), std::ctime(&cftime), _filename };
}

std::size_t RegularFile::size() const
{
    return fs::file_size(_filename);
}

std::string RegularFile::filename() const
{
    return _filename;
}

FileInfo::PermisionsT RegularFile::permision() const
{
    bool read = false;
    bool write = false;
    if ( ( _perms & fs::perms::owner_read ) == fs::perms::none )
        read = true;
    if ( ( _perms & fs::perms::owner_write ) == fs::perms::none )
        write = true;

    if ( read && write )
        return FileInfo::RW;
    else if ( read )
        return FileInfo::READ;
    else
        return FileInfo::WRITE;
}


void RegularFile::setPermision(Perms perms)
{
    if ( perms == Perms::READ )
        _perms |= fs::perms::owner_read | fs::perms::others_read | fs::perms::group_read;
    else if ( perms == Perms::WRITE )
        _perms |= fs::perms::owner_write;
    else
        _perms |= fs::perms::owner_write | fs::perms::owner_read | fs::perms::others_read | fs::perms::group_read;

    fs::permissions( _filename, _perms );
    _access = true;
}

void RegularFile::disableWrite()
{
    // currentPermissions & ~fs::perms::owner_write & ~fs::perms::group_write & ~fs::perms::others_write
    _perms = ( _perms & ~fs::perms::owner_write );
    fs::permissions( _filename, _perms );
}

void RegularFile::disableRead()
{
    _perms = ( _perms & ~fs::perms::owner_read );
    fs::permissions( _filename, _perms );
}

void RegularFile::disableAll()
{
    std::lock_guard<std::mutex> lk(_mutex);
    _access = false;
};

}
