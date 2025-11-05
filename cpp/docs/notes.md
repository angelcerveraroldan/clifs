Here are some notes that I took down as I was working on the project.

They may not be right, or make much sense for someone else reading it! It is mainly meant
for myself, and will likely be deleted later.

# Basic usage

Run the binary with some path to mount. For exaple, `./build/clifs src` (likely not a good idea to do this on the project directory).
After running this, run `l` and you should see something like:

```
drwxrwxr-x 8 angelcr angelcr 4.0K Sep 29 15:49 .
drwxrwxr-x 4 angelcr angelcr 4.0K Sep 25 16:59 ..
drwxrwxr-x 3 angelcr angelcr 4.0K Sep 29 15:43 build
drwxrwx--- 3 angelcr angelcr 4.0K Sep 26 10:43 .cache
-rw-rw-r-- 1 angelcr angelcr  618 Sep 26 10:38 CMakeLists.txt
drwxr-xr-x 2 angelcr angelcr 4.0K Sep 29 15:50 docs
drwxrwxr-x 8 angelcr angelcr 4.0K Sep 26 10:45 .git
-rw-r--r-- 1 angelcr angelcr   13 Sep 26 10:45 .gitignore
drwxr-xr-x 2 angelcr angelcr 4.0K Sep 26 10:33 include
-rw-rw-r-- 1 angelcr angelcr  246 Sep 26 10:43 makefile
d????????? ? ?       ?          ?            ? src
```

Now, you can unmount the directory so that it functions normally using `fusermount -u src`.
