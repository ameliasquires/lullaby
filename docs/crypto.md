# crypto

## hashing

\* is optional

|name|out len|other args|extra|
|--|--|--|--|
| sha0 | 160 | nil | insecure, use sha1|
| sha1 | 160 | nil | |
| sha256 | 256 | nil | |
| sha224 | 224 | nil | |
| pearson | 8 | nil | use setpearson(table) to change the table, initial is 0..255|
| xxh64 | 64 | nil | xxhash |
| xxh32 | 32 | nil | |
| crc8 | 8 | nil | |
| crc16 | 16 | nil | |
| crc32 | 32 | nil | |
| fletcher8 | 8 | nil | |
| fletcher16 | 16 | nil | |
| fletcher32 | 32 | nil | |
| sysvchecksum | 32 | nil | |
| xor8 | 8 | nil | |
| buzhash8 | 8 | nil | use setbuzhash(table) to change table (will affect all buzhash functions) |
| buzhash16 | 16 | nil | ^ |
| cityhash32 | 32 | nil | |
| cityhash64 | 64 | nil | |
| cityhash128 | 128 | nil | |
| md5 | 128 | nil | |
| djb2 | 64 | nil | |
| farmhash32 | 32 | nil | |
| farmhash64 | 64 | nil | |
| fasthash32 | 32 | *seed | |
| fasthash64 | 64 | *seed | |
| fnv_0 | 64 | nil | |
| fnv_1 | 64 | nil | |
| fnv_a | 64 | nil | |
| oaat | 32 | nil | |
| lostlose | 64 | nil | |
| metrohash64_v1 | 64 | *seed | |
| metrohash64_v2 | 64 | *seed | |
| metrohash128_v1 | 128 | *seed | |
| metrohash128_v2 | 128 | *seed | |
| murmur1_32 | 32 | *seed | |
| murmur2_32 | 32 | *seed | |
| pjw | 32 | *seed | |
| sdbm | 64 | nil | |
| sha512 | 512 | nil | |
| sha384 | 384 | nil | |
| sha512_t | length of arg 2 | t (bit length) | bit length range is 0 < t <= 512 (this isnt checked, and it should accept any value) |
| spookyhash128_v1 | 128 | *seed | |
| spookyhash128_v2 | 128 | *seed | |
| spookyhash64_v1 | 64 | *seed | |
| spookyhash64_v2 | 64 | *seed | |
| spookyhash32_v1 | 32 | *seed | |
| spookyhash32_v2 | 32 | *seed | |
| blake2b | length of arg 2 * 8 | *output len (default is 64), *key | |
| blake2s | length of arg 2 * 8 | *output len (default is 32), *key | |

### usage

```lua
llib.crypto.sha512("meow") -- e88348269bad036160f0d9558b7c5de68163b50e1a6ce46e85ee64692eba074529a4a2b48db4d5c36496e845001e13e6d07c585eacd564defcbf719ec9033e17 
llib.crypto.sha512_t("meow", 224) -- would be sha512/224 - ad5e403e0d74532187f4e1665c7e705ab5eb3c2fe07ae73a3ff998b2
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
