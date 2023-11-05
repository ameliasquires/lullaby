require "llib"

function test(name,b,exp)
  local hash = llib.crypto[name](b)
  if not (hash == exp) then
    print(name.." not working, got "..hash)
  end
end

test("adler32","meow","043c01b9")
test("bsdchecksum","meow","24789")
test("crc8","meow","a4")
test("crc16","meow","6561")
test("crc32","meow","8a106afe")
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


