#include <chrono>
#include <ctime>
#include <filesystem>
#include <ios>
#include "vfs/RegularFile.h"
#include "vfs/IFS.h"
#include "vfs/IFile.h"

namespace VFS {

namespace fs = std::filesystem;

RegularFile::RegularFile(std::string const & filename)
    : _filename(filename)
      , _outStream(_filename, std::ios::out | std::ios::binary)
      , _inStream(_filename, std::ios::in | std::ios::binary)
      , _access(false)
      , _perms()
{
    if ( fs::exists(_filename) && ( _outStream.is_open() && _inStream.is_open() ) )
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
    if ( !_access || ( _perms & fs::perms::owner_write ) == fs::perms::none )
        return -1;

    auto beginPos = _outStream.tellp();
    _outStream.write(buf.data(), size);
    if ( _outStream.fail() )
        return -1;
    auto endPos = _outStream.tellp();

    auto wrote = endPos - beginPos;
    return wrote;
}

std::size_t RegularFile::write(Buffer const & buf, std::size_t offset, std::size_t size)
{
    if ( !_access || ( _perms & fs::perms::owner_write ) == fs::perms::none )
        return -1;

    _outStream.seekp(offset, std::ios::beg);
    auto wroteSize = write(buf, size);
    _outStream.seekp(0, std::ios::end);

    return wroteSize;
}

RegularFile::Buffer RegularFile::read(std::size_t size)
{
    if ( !_access || ( _perms & fs::perms::owner_read ) == fs::perms::none )
        return Buffer();

    return read(0, size);
}

RegularFile::Buffer RegularFile::readAll()
{
    if ( !_access || ( _perms & fs::perms::owner_read ) == fs::perms::none )
        return Buffer();

    auto totalSize = size();

    return read(0, totalSize);
}

RegularFile::Buffer RegularFile::read(std::size_t offset, std::size_t size)
{
    if ( !_access || ( _perms & fs::perms::owner_read ) == fs::perms::none )
        return {};

    auto totalSize= fs::file_size(_filename);
    _inStream.seekg(offset, std::ios::beg);
    auto validSize = ( size < totalSize ? size : totalSize );
    Buffer buf(validSize, '.');
    _inStream.read(buf.data(), validSize);

    if ( _inStream.fail() )
        return {};

    return buf;
}

void RegularFile::close()
{
    if ( !_access )
        return;

    _outStream.flush();
    _outStream.close();
    _inStream.close();

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

}
