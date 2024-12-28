#ifndef IFILE_H
#define IFILE_H

#include <cstddef>
#include <string>
#include <vector>
#include "FileInfo.h"
#include "global.h"

namespace VFS {

enum Perms : char
{
    READ = 1,
    WRITE = 2,
    RW = 4,
};

class IFile
{
public:
    typedef char DataT;
    typedef std::vector<DataT> Buffer;

public:
    IFile() = default;
    virtual ~IFile() = default;

    DISABLE_COPY(IFile)

    /**
     * @brief Write data to this file start positon that file pointer pointed to.
     The implementation need to consider the data race problem to ensure thread-safe.
     * 
     * @param buf - buffering all data
     * @param size - wrote to this file
     * @return std::size_t - successfully wrote size
     */

    virtual std::size_t write(Buffer const & buf, std::size_t size) = 0;

    /**
     * @brief Write data to this file. The start position is the offset relative to the start position of the file.
     * 
     * @param buf - buffering all data
     * @param offset - relative to the start position of the file
     * @param size - wrote to this file
     * @return std::size_t - successfully wrote size
     */
    virtual std::size_t write(Buffer const & buf, std::size_t offset, std::size_t size) = 0;

    /**
     * @brief Read data from the file start position that file pointer pointed to.
     The operation have not data race problem.
     * 
     * @param size - need to read
     * @return Buffer - buffering all data that read from this file
     */
    virtual Buffer read(std::size_t size) = 0;

    /**
     * @brief Read data from the offset position relative to the start position of the file.
     * 
     * @param offset - relative to the start position of the file
     * @param size - need to read
     * @return Buffer - buffering all data that read from this file
     */
    virtual Buffer read(std::size_t offset, std::size_t size) = 0;

    virtual Buffer readAll() = 0;

    virtual void close() = 0;

    /**
     * @brief Get information about this file, containing total size, file type, access control permissions,
     modified time and filename and so on.
     * 
     * @return FileInfo
     */
    virtual FileInfo info() const = 0;

    virtual std::size_t size() const = 0;

    /**
     * @brief The filename is meeting POXIS standards.
     * 
     * @return std::string
     */
    virtual std::string filename() const = 0;

    /**
     * @brief Meeting POXIS standards.
     * 
     * @return FileInfo::PermisionsT - c-style string, either "--", or "r-", or "rw"
     */
    virtual FileInfo::PermisionsT permision() const = 0;

    virtual void setPermision(Perms perms) = 0;

    virtual void disableWrite() = 0;

    virtual void disableRead() = 0;

    virtual void disableAll() = 0;
};

} // namespace VFS

#endif // !VFS_H
