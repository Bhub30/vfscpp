#include <chrono>
#include <ctime>
#include <filesystem>
#include <ios>
#include "RegularFile.h"
#include "IFile.h"

namespace VFS {

namespace fs = std::filesystem;

RegularFile::RegularFile(std::string const & filename)
    : _filename(filename)
      , _stream(_filename, std::ios::out | std::ios::binary)
      , _inStream(_filename, std::ios::in | std::ios::binary)
      , _access(false)
      , _size(0)
      , _perms()
{
    if ( fs::exists(_filename) && ( _stream.is_open() && _inStream.is_open() ) )
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

    auto beginPos = _stream.tellp();
    _stream.write(buf.data(), size);
    if ( _stream.fail() )
        return -1;
    auto endPos = _stream.tellp();

    size = endPos - beginPos;

    if ( size )
        _size += size;

    return size;
}

std::size_t RegularFile::write(Buffer const & buf, std::size_t offset, std::size_t size)
{
    if ( !_access || ( _perms & fs::perms::owner_write ) == fs::perms::none )
        return -1;

    _stream.seekp(offset, std::ios::beg);

    auto wroteSize = write(buf, size);
    _stream.seekp(0, std::ios::end);

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

    auto totalSize = this->size();
    _inStream.seekg(offset, std::ios::beg);
    auto validSize = ( size < totalSize ? size : totalSize );
    Buffer buf{};
    buf.reserve(validSize);
    _inStream.read( buf.data(), validSize );
    _inStream.seekg(0, std::ios::end);

    if ( _inStream.fail() )
        return {};

    return buf;
}

void RegularFile::close()
{
    if ( !_access )
        return;

    _stream.flush();
    _stream.close();
    _inStream.close();

    _access = false;
}

FileInfo RegularFile::info() const
{
    using namespace std::chrono;

    auto fileTime = fs::last_write_time(_filename);
    auto sctp = time_point_cast<system_clock::duration>(fileTime - fs::file_time_type::clock::now() + system_clock::now());
    auto cftime = system_clock::to_time_t(sctp);

    return { "RegualFile", permision(), _size, std::ctime(&cftime), _filename };
}

std::size_t RegularFile::size()
{
    if ( _size == 0 )
        _size = static_cast<std::size_t>( fs::file_size(_filename) );

    return _size;
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
        fs::permissions( _filename, _perms | fs::perms::owner_read );
    else if ( perms == Perms::WRITE )
        fs::permissions( _filename, _perms | fs::perms::owner_write );
    else
        fs::permissions( _filename, _perms | fs::perms::owner_read | fs::perms::owner_write );
}

void RegularFile::disableWrite()
{
    // currentPermissions & ~fs::perms::owner_write & ~fs::perms::group_write & ~fs::perms::others_write
    fs::permissions( _filename, _perms & ~fs::perms::owner_write );
}

void RegularFile::disableRead()
{
    fs::permissions( _filename, _perms & ~fs::perms::owner_read );
}

}
