# About

This is a C++ Virtual File System library demo that allows manipulation of files from memory or native filesystems. My purpose to build this is used to Network File System. It is just so simple and single. The future maybe build with more new feature.

> Note: This library requires C++17 or later

## Example

You can construct a FileSystem object with certain path, and then it automatly mount the filesystem:

```c++
#include "vfs/VFS.h"

auto fsDir =  "/path/to";
VFS::FileSystem fs( fsDir );
```

You only can mount other path in the same object after unmounting current path:

```c++
fs.unmount();
fs.mount("/other/path/to");
```

It's often useful to construct with smart pointer:

```c++
VFS::IFS fs1(new FileSystem("/path/to"));
VFS::IFS fs2 = std::make_share<FileSystem>("/path/to/some")
```

More example about file operation can be found in unit test.
