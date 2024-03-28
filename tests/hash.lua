require "llib"

local hashes_working = 0
local hashes_failed = 0
local functions_working = 0
local functions_failed = 0

function test(name,b,exp,oargs)
  local fail = false
  local hash
  local hash2
  local hash3
  local hash4
  local add = ""
  if oargs == nil then
    hash = llib.crypto[name](b)
  else
    hash = llib.crypto[name](b,table.unpack(oargs))
    add = table.concat(oargs, ", ")
  end

  if(llib.crypto[name.."_init"] ~= nil) then
    hash2 = llib.crypto[name]():update(b):final()

    hash3 = llib.crypto[name]()
    b:gsub(".", function(c) hash3:update(c) end)
    hash3 = hash3:final()
    
    hash4 = llib.crypto[name.."_init"]()
    llib.crypto[name.."_update"](hash4, b)
    hash4 = llib.crypto[name.."_final"](hash4)

    if(hash2 ~= hash) then
      fail = true
      functions_failed=functions_failed + 1
      llib.io.error(name.." alt method not working, got:\n\t"..hash2.." other was:\n\t"..hash)
    else 
      functions_working=functions_working + 1
      llib.io.log(name.." alt method working "..hash2.." == "..hash)
    end

    if(hash4 ~= hash) then
      fail = true
      functions_failed=functions_failed + 1
      llib.io.error(name.." alt method 2 not working, got:\n\t"..hash4.." other was:\n\t"..hash)
    else 
      functions_working=functions_working + 1
      llib.io.log(name.." alt method 2 working "..hash4.." == "..hash)
    end

    if(hash3 ~= hash) then
      fail = true
      functions_failed=functions_failed + 1
      llib.io.error(name.." alt char-b-char method not working, got:\n\t"..hash3.." other was:\n\t"..hash)
    else 
      functions_working=functions_working + 1
      llib.io.log(name.." alt char-b-char method working "..hash3.." == "..hash)
    end

  end

  if not (hash == exp) then
    fail = true
    functions_failed=functions_failed + 1
    llib.io.error(name.." not working, got:\n\t"..hash.." wanted:\n\t"..exp.."\n\twith args: {"..add.."}")
  else
    functions_working=functions_working + 1
    llib.io.log(name.." was correct, "..hash)
  end

  if(fail) then
    hashes_failed=hashes_failed + 1
  else 
    hashes_working=hashes_working + 1
  end

end

test("adler32","meow","043c01b9")
test("bsdchecksum","meow","24789")
test("crc8","meow","a4")
test("crc16","meow","6561")
test("crc32","meow","8a106afe")
test("fletcher8","meow","05")
test("fletcher16","meow","3cb9")
test("fletcher32","meow","043801b8")
test("md5","meow","4a4be40c96ac6314e91d93f38043a634")
test("pearson","meow","10")
test("sha0","meow","36a22def8a9e92a1ee73579abc389e8a21b24b61")
test("sha1","meow","7d5c2a2d6136fbf166211d5183bf66214a247f31")
test("sha224","meow","e28f8ee4dd8618b890df366b85a2d45d2506dd842e95272b9a598998")
test("sha256","meow","404cdd7bc109c432f8cc2443b45bcfe95980f5107215c645236e577929ac3e52")
test("sysvchecksum","meow","1b8")
test("xor8","meow","48")
test("xxh32","meow","6ba6f6f0")
test("xxh64","meow","bc11093a30a6315f")
test("buzhash8","meow","57")
test("buzhash16","meow","0255")
test("cityhash32","meow","c41a03e9")
test("cityhash64","meow","e99b592ae1ff868b")
test("cityhash128","meow","d73f2b9c5501a6524097c5d815f2152")
test("djb2","meow","7c9a913d")
test("farmhash32","meow","c41a03e9");
test("farmhash64","meow","e99b592ae1ff868b")
--maybe test fasthash, metrohash, sha512_t and murmur blehh
test("fnv_0","meow","b0850402171532ac")
test("fnv_1","meow","c60a427ebfe83be5")
test("fnv_a","meow","42faffa2e30e025d")
test("oaat","meow","8532510")
test("loselose","meow","000001b8")
test("pjw","meow","00073c67")
test("sdbm","meow","006d50f201921b00")
test("sha512","meow","e88348269bad036160f0d9558b7c5de68163b50e1a6ce46e85ee64692eba074529a4a2b48db4d5c36496e845001e13e6d07c585eacd564defcbf719ec9033e17");
test("sha384","meow","f0bb848a382b5ed5e2f49a46252f6b738c933dc20bb29dc4a5d312e310b395c4fa07f30a8a7380b4a5d367445e0ea8cb")
test("fasthash64","meow","7b9e494cf11ee113")
test("fasthash32","meow","758097c7")
test("metrohash64_v1", "meow", "7435945e80261ed1")
test("metrohash64_v2","meow","f951647d250e36f0")
test("metrohash128_v1","meow","bfd8835cbcc06d2be6fc2c8e5ecbcc26")
test("metrohash128_v2","meow","6d8634ccf529269297704cba8bf8707a")
test("murmur1_32","meow","743df82f")
test("murmur2_32","meow","05d01b88")
test("blake2b","meow","9919ae53fbea6c5da68e51b6e19a890fdbc01baf97fff29efd7efaa7163ea7aa205109b818bde29da815e16b869dbb2cb1b367ed1027f52116287d760808a43d")
test("blake2b","meow","424969d2fe47cdec2a824709b8066cc1d63cc4b6a16a3c1fa421cc2a6625f7c2",{32})
test("blake2b","meow","6f30fdec70f9ed6d8db2e7407d3e2325af23935464ec3ec1cf4c12575ff3c18043bf772033b91d52978c451d01f7eaeacabda76460b9f4b7bf516dd9d0cc886d",{64,"owo"})
test("blake2s","meow","f461bed24c982ccb29cb967acdaebc9494b51c1d0f88f6bc47850952261a512d")
test("blake256", "meow", "067805dd21a4ef97460c6613f231437917a1c1c7f1dcd1bfe67d952d09ccb028")
test("blake224", "meow", "0a099d692027cfbe69d2424a5b2520a7398fa4945e0878f6c541f5ce")
test("blake512", "meow", "09d5abe166c4ad855d4527d0be21df2b1a01c3d7c5637572561ebc247908fd7db30bf342391dd0a834fd35f391807480fb31e8a7ee3b1098e46d996d5601948f")
test("blake384", "meow", "7edb2ff04616f5551a789217029496c3c8601ac7aba2d40d7fcd1ec85fc63f37514e5884f2ebc807b11854247620446c")

print(hashes_working.."/"..hashes_failed.." hashes working/failed")
print(functions_working.."/"..functions_failed.." functions working/failed")