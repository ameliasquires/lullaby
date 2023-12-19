require "llib"
llib.config.set({file_chunksize = 2})
print(llib.io.readfile("src/hash/md5.c"))
