# CliFS ⛰️

A minimal in-memory File System written in C++17.

# Implemented operations

The following FUSE operations are currently implemented:

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

# Project Structure

```
makefile
CMakeLists.txt
include
├── clifs_tree.h
├── file_desc.h
├── params.h
└── state.h
README.md
src
├── clifs_tree.cpp
├── file_desc.cpp
└── main.cpp
```

*`clifs_tree`:* This is a tree containing the information of the directories and the files. This is where a lot of the functionality is implemented, from making new files and directories as well as deleting them, to storing their basic metadata (permissions, owner, size, links).

*`file_desc`:* This data structure will keep track of what files are currently open. It will assign each open file an id, which fuse will use to write, read, ...

*`main`:* This is where the FUSE operations are implemented. Inside the main function, `fuse_main` is executed, which mounts the file system.

# Running

First generate the binary: `make build`

Then create a root directory which we will later mount to this FS: `mkdir mount_point`

Now mount that directory: `./build/clifs mount_point`

If you now run `ls`, you should see the `mount_point` directory, `cd` into it. Now any commands you use (i.e. `touch`) will use the CliFS implementation rather than your real disk.

When you are finished, make sure to unmount the directory `fusermount -u mount_point`. After doing this, if you `cd` into `mount_point`, you will notice that anything you created with CliFS is no longer there.

# Todos

- Save file/dir creation time
- Unit testing
- Add better error handling!
- Persist files
