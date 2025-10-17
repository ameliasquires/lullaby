# crypto

> out of date!

## hashing

\* is optional

|name|out len|other args|extra|incremental|
|--|--|--|--|--|
| adler32 | 32 | nil | | y |
| bsdchecksum | 16 | nil | | y |
| sha0 | 160 | nil | insecure, use sha1| y |
| sha1 | 160 | nil | | y |
| sha256 | 256 | nil | | y |
| sha224 | 224 | nil | | y |
| pearson | 8 | nil | use setpearson(table) to change the table, initial is 0..255| y |
| xxh64 | 64 | nil | xxhash | todo|
| xxh32 | 32 | nil | | todo |
| crc8 | 8 | nil | | y |
| crc16 | 16 | nil | | y |
| crc32 | 32 | nil | | y |
| fletcher8 | 8 | nil | | y |
| fletcher16 | 16 | nil | | y |
| fletcher32 | 32 | nil | | y |
| sysvchecksum | 32 | nil | | y |
| xor8 | 8 | nil | | y |
| buzhash8 | 8 | nil | use setbuzhash(table) to change table (will affect all buzhash functions)| n |
| buzhash16 | 16 | nil | ^ | n |
| cityhash32 | 32 | nil | | n |
| cityhash64 | 64 | nil |  | n |
| cityhash128 | 128 | nil |  | n |
| md5 | 128 | nil | | y |
| djb2 | 64 | nil | | y |
| farmhash32 | 32 | nil | | n |
| farmhash64 | 64 | nil | | n |
| fasthash32 | 32 | *seed | | n |
| fasthash64 | 64 | *seed | | n |
| fnv_0 | 64 | nil | | y |
| fnv_1 | 64 | nil | | y |
| fnv_a | 64 | nil | | y |
| oaat | 32 | nil | | y |
| loselose | 64 | nil | | y |
| metrohash64_v1 | 64 | *seed | | n |
| metrohash64_v2 | 64 | *seed | | n |
| metrohash128_v1 | 128 | *seed | | n |
| metrohash128_v2 | 128 | *seed | | n |
| murmur1_32 | 32 | *seed | | n |
| murmur2_32 | 32 | *seed | | n |
| pjw | 32 | nil | | y |
| sdbm | 64 | nil | | y |
| sha512 | 512 | nil | | y |
| sha384 | 384 | nil | | y |
| sha512_t | length of arg 2 | t (bit length) | bit length range is 0 < t <= 512 (this isnt checked, and it should accept any value) | y |
| spookyhash128_v1 | 128 | *seed | | n |
| spookyhash128_v2 | 128 | *seed | | n |
| spookyhash64_v1 | 64 | *seed | | n |
| spookyhash64_v2 | 64 | *seed | | n |
| spookyhash32_v1 | 32 | *seed | | n |
| spookyhash32_v2 | 32 | *seed | | n |
| blake2b | length of arg 2 * 8 | *output len (default is 64), *key | | y |
| blake2s | length of arg 2 * 8 | *output len (default is 32), *key | | y |
| blake256 | 256 | nil | | y |
| blake224 | 224 | nil | | y |
| blake512 | 512 | nil | | y |
| blake384 | 384 | nil | | y |

### usage

```lua
llib.crypto.sha512("meow") -- e88348269bad036160f0d9558b7c5de68163b50e1a6ce46e85ee64692eba074529a4a2b48db4d5c36496e845001e13e6d07c585eacd564defcbf719ec9033e17 
llib.crypto.sha512_t("meow", 224) -- would be sha512/224 - ad5e403e0d74532187f4e1665c7e705ab5eb3c2fe07ae73a3ff998b2
```

the + operator clones the hash and returns it so the orignal is not modified,
when using a non-cloning method, make sure to redefine the hash object to the updated value.
not doing this can ruin the sync in the hash

functions supporting updates (see above) can be used like so:

```lua
obj = llib.crypto.adler32() --adler32_init is equivilant to adler32 with no params
obj = llib.crypto.adler32_update(obj, "meow")
local hash = llib.crypto.adler32_final(obj) --043c01b9

--or you can chain them!
obj = llib.crypto.adler32()
obj = obj:update("meow")
hash = obj:final() --043c01b9s (the same)

--along with the + operator being overloaded to work as obj:update and returning a seperate object
obj = llib.crypto.adler32()
hash = (obj + "meow"):final() -- (the same again)
hash = obj:update("meow"):final() -- you get the point

--and of course, the single function method still works too (will still do init-update-final in the backend)
hash = llib.crypto.adler32("meow") --043c01b9s (the same)

--for any extra arguments, they should be used in the init function
llib.crypto.sha512_t_init(224) -- like example above
--or even (only if the arguments are required)
llib.crypto.sha512_t(224) -- same as above
```

## en/decoding

all functions have 1 argument which is a string, unless noted otherwise

|name|encode|decode|notes|
|--|--|--|--|
|uuencode|uuencode|uudecode| |
|base64|base64encode|base64decode| |

### usage

```lua
llib.crypto.base64encode("purr") -- cHVycg==
llib.crypto.base64decode("cHVycg==") -- purr
```

## baseconvert 

'accepts an array of integers

converts an array from base N to base T (in reversed order)

```lua
--                      input                           N  T
llib.crypto.baseconvert({1, 1, 0, 1, 0, 1, 0, 0, 0, 1}, 2, 10) -- {9, 4, 8} (which is 849)
```
