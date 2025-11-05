# CliFS

A minimal in-memory File System implemented in both C++17 and in C.

This was a project intended to learn the programming languages. See `cpp/` for the C++ version, and `c/` for the C version.

# Implemented operations

The following FUSE operations are currently implemented in the C++ version, and soon to be in the C version:

- `getattr`
- `readdir`
- `mkdir`
- `rename`
- `rmdir`
- `open`
- `read`
- `release`
- `mknod`
- `unlink`
- `write`

With the above FUSE operations, you can use all the basic commands (`ls`, `cat`, `touch`, `mkdir`, `cd`, ...). You can also mostly edit files with NeoVim, but due to some incomplete operations, it will sometimes crash!


