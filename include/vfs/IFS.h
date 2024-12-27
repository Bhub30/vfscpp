#ifndef FS_H    // filesystem interface
#define FS_H

#include <memory>
#include <string>
#include <vector>
#include "IFile.h"
#include "global.h"

namespace VFS {

namespace type {
    typedef char const * const FILETYPE;
    constexpr static char const * const PIPE        = "PIPE";
    constexpr static char const * const UNKOWN      = "UNKOWN";
    constexpr static char const * const SOCKET      = "SOCKET";
    constexpr static char const * const SYMLINK     = "SYMLINK";
    constexpr static char const * const DIRECTORY   = "DIRECTORY";
    constexpr static char const * const REGULAR     = "REGUAL FILE";
    constexpr static char const * const BLOCK       = "BLOCK DEVICE";
    constexpr static char const * const NOTFOUND    = "DOES NOT EXIST";
    constexpr static char const * const NONE        = "NOT-EVALUATED-YET TYPE";
    constexpr static char const * const IMPLDEFINE  = "IMPLEMENTATION-DEFINED TYPE";
    // 
} // namespace type

class IFS
{
public:
    typedef std::shared_ptr<IFile> IFilePtr;
    typedef std::vector<std::string> EntryList;
    typedef std::shared_ptr<IFS> IFSPtr;

public:
    IFS() = default;
    DISABLE_COPY(IFS);
    virtual ~IFS() = default;

    virtual std::string path() const = 0;

    virtual bool isMounted() const = 0;

    /**
     * @brief Mount exact filesystem. If the specified path is not exist, then return false, else true.
     * 
     * @param path - the path for filesystem according to POXIS
     * @return true - the path is excat existed.
     * @return false - the path is not existed.
     */
    virtual bool mount(std::string const & path) = 0;

    /**
     * @brief Unmount the mounted filesystem. If the filesystem is not mounted, then return false, else true.
     * 
     * @return true - the specify path is already mounted.
     * @return false - the specify path is not mounted.
     */
    virtual bool unmount() = 0;

    /**
     * @brief Open a file acorrding to the specified path and filename. The default mode is only read. The file must
     * already exists.
     *
     * @param filename - relative paht to the filesystem meeting POSIX standards, only regular file
     * @param mode - access control, default READ
     * @return IFilePtr - the pointer to the open file, or nullptr if is a directory
     */
    virtual IFilePtr open(std::string const & filename, Perms mode = Perms::RW) = 0;

    /**
     * @brief Delete the specified file from the mounted filesystem. It is worth noting that the file maybe already open,
     so the implemeatation need to judge the proper time for deleting the file.
     * 
     * @param filename - relative paht to the filesystem meeting POSIX standards, regulare file or directory
     * @return true - success to delete the file
     * @return false - the file maybe not exist
     */
    virtual bool remove(std::string const & filename) = 0;

    /**
     * @brief Touch a new file in the mounted filesystem. If the file is already exist, then do nothing and return false
     * 
     * @param filename - relative paht to the filesystem meeting POSIX standards, only regular file
     * @return true - success
     * @return false - maybe already existed
     */
    virtual bool touchFile(std::string const & filename) = 0;

    /**
     * @brief Make a new directory.
     *
     * @param dir - name
     * @return true - successfully make
     * @return false - failed to make, maybe already exist or has invalid directory name
     */
    virtual bool makeDir(std::string const & dir) = 0;

    /**
     * @brief The specified file or directory will be move to other location inside the filesystem. The location must legal to POXIS.
     *
     * @param from - relative path to the mounted filesystem
     * @param to - relative path to the mounted filesystem
     * @return true - success
     * @return false - the original file not exist or the target already exists
     */
    virtual bool moveTo(std::string const & from, std::string const & to) = 0;

    /**
     * @brief Move the file to the specified mounted filesystem.
     *
     * @param from - relative path to this filesystem
     * @param fs - other already mounted filesystem
     * @param to - relative path to the target filesystem
     * @return true - success
     * @return false - the target filesystem maybe not correct or exist
     */
    virtual bool moveTo(std::string const & from, IFSPtr fsptr, std::string const & to) = 0;

    /**
     * @brief List all files that contain sub-directories in top level of the filesysten.
     Those sub-directory contained in the return resull just returned the directory name and then not contain the contents in the sub-direcoty.
     *
     * @return EntryList - vector that contain all files name on top level of ths filesystem
     */
    virtual EntryList list() = 0;

    /**
     * @brief List all files that contain sub-directories in top level of the specified directory.
     Those sub-directory contained in the return resull just returned the directory name and then not contain the contents in the sub-direcoty.
     * 
     * @param dir - absolute path relative to the mounted filesystem
     * @return EntryList - vector that contain all files name inside the directory
     */
    virtual EntryList list(std::string const & dir) = 0;

    /**
     * @brief Find the target file in the filesystem.
     * 
     * @param filename - meeting POSIX standards, both absoulte and relative path is ok
     * @return true - the target file is exist
     * @return false - the target file is not exist
     */
    virtual bool contain(std::string const & filename) = 0;

    /**
     * @brief Search the specified file or directory that maybe relative or absolute path according to the filesystem.
     * If exist, returned the relative path according to the filesystem
     *
     * @param filename - file or directory
     * @param string - if exist is the relative path, else the type::NOFOUND
     */
    virtual std::string search(std::string const & filename) = 0;

    /**
     * @brief Perform the copy operation for the specified file. Can't come cross different filesystem
     * 
     * @param from - meeting POSIX standards
     * @param to - target location that locate in the filesystem
     * @return true - success to copy
     * @return false - the file maybe delete from the filesystem or not exist
     */
    virtual bool copy(std::string const & from, std::string const & to) = 0;

    virtual type::FILETYPE type(std::string const & filename) = 0;
};

} // namespace VFS

#endif // !FS_H
