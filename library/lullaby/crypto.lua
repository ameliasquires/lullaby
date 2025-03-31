---@meta

---@class crypto
local crypto = {}

---@class hash
---@operator add(string): hash 
local hash = {}

---@param T hash
---@param value string
function hash.update(T, value) end

---@param T hash
function hash.final(T) end

---@return hash
---@overload fun(value: string): string
function crypto.adler32() end

---@return hash
---@overload fun(value: string): string
function crypto.bsdchecksum() end

---@return hash
---@overload fun(value: string): string
function crypto.sha0() end

---@return hash
---@overload fun(value: string): string
function crypto.sha1() end

---@return hash
---@overload fun(value: string): string
function crypto.sha256() end

---@return hash
---@overload fun(value: string): string
function crypto.sha224() end

---@return hash
---@overload fun(value: string): string
function crypto.pearson() end

---@param T integer[]
function crypto.setpearson(T) end

---@param input string
---@return string
function crypto.xxh64(input) end

---@param input string
---@return string
function crypto.xxh32(input) end

---@return hash
---@overload fun(value: string): string
function crypto.crc8() end

---@return hash
---@overload fun(value: string): string
function crypto.crc16() end

---@return hash
---@overload fun(value: string): string
function crypto.crc32() end

---@return hash
---@overload fun(value: string): string
function crypto.fletcher8() end

---@return hash
---@overload fun(value: string): string
function crypto.fletcher16() end

---@return hash
---@overload fun(value: string): string
function crypto.fletcher32() end

---@return hash
---@overload fun(value: string): string
function crypto.sysvchecksum() end

---@return hash
---@overload fun(value: string): string
function crypto.xor8() end

---@param input string
---@return string
function crypto.buzhash8(input) end

---@param input string
---@return string
function crypto.buzhash16(input) end

---@param T integer[]
function crypto.setbuzhasah(T) end

---@param input string
---@return string
function crypto.cityhash32(input) end

---@param input string
---@return string
function crypto.cityhash64(input) end

---@param input string
---@return string
function crypto.cityhash128(input) end

---@return hash
---@overload fun(value: string): string
function crypto.md5() end

---@return hash
---@overload fun(value: string): string
function crypto.djb2() end

---@param input string
---@return string
function crypto.farmhash32(input) end

---@param input string
---@return string
function crypto.farmhash64(input) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.fasthash32(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.fasthash64(input, seed) end

---@return hash
---@overload fun(value: string): string
function crypto.fnv_0() end

---@return hash
---@overload fun(value: string): string
function crypto.fnv_1() end

---@return hash
---@overload fun(value: string): string
function crypto.fnv_a() end

---@return hash
---@overload fun(value: string): string
function crypto.oaat() end

---@return hash
---@overload fun(value: string): string
function crypto.loselose() end

---@param input string
---@param seed integer | nil
---@return string
function crypto.metrohash64_v1(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.metrohash64_v2(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.metrohash128_v1(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.metrohash128_v2(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.murmur1_32(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.murmur2_32(input, seed) end

---@return hash
---@overload fun(value: string): string
function crypto.pjw() end

---@return hash
---@overload fun(value: string): string
function crypto.sdbm() end

---@return hash
---@overload fun(value: string): string
function crypto.sha512() end

---@return hash
---@overload fun(value: string): string
function crypto.sha384() end

---@param t integer 0 < t <= 512
---@return hash
---@overload fun(t, value: string): string
function crypto.sha512_t(t) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.spookyhash128_v1(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.spookyhash128_v2(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.spookyhash64_v1(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.spookyhash64_v2(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.spookyhash32_v1(input, seed) end

---@param input string
---@param seed integer | nil
---@return string
function crypto.spookyhash32_v2(input, seed) end

---@return hash
---@overload fun(value: string): string
---@overload fun(value: string, length: integer, key: string): string
---@overload fun(length: integer, key: string): hash
function crypto.blake2b() end

---@return hash
---@overload fun(value: string): string
---@overload fun(value: string, length: integer, key: string): string
---@overload fun(length: integer, key: string): hash
function crypto.blake2s() end

---@return hash
---@overload fun(value: string): string
function crypto.blake256() end

---@return hash
---@overload fun(value: string): string
function crypto.blake224() end

---@return hash
---@overload fun(value: string): string
function crypto.blake512() end

---@return hash
---@overload fun(value: string): string
function crypto.blake384() end

return crypto
