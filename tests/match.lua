llb = require "lullaby"

function test(a, b, match, expect)
  res, out = llb.test._match(a, b)

  if res ~= match then
    return llb.io.error("res != match")
  end

  if res == 0 then return end
  
  if llb.array.len(out) ~= llb.array.len(expect) then
    return llb.io.error("out != expect")
  end

  for i, v in ipairs(expect) do
    if v ~= out[i] then
      return llb.io.error("out != expect")
    end
  end
  llb.io.pprint(out)
end

test("/{test}/","/name/", 1, {test="name"})
test("*","/wdejowe/wde", 1, {})
test("/*/{hello}/{meow}/end","/blah/blah/hii/end", 1, {hello="blah", meow="hii"})
test("/*/*/{test}/*/*/end/*/real","/a/b/testing/d/e/end/f/real", 1, {test="testing"})
test("/*/a/b/end/{word}","/w/o/m/p/a/b/end/meow", 1, {word="meow"})
test("*", "meow/meow/meow", 1, {})
test("{meow}", "owo", 1, {meow="owo"})
test("/{meow}", "/owo", 1, {meow="owo"})
test("{meow}", "/", 0)
test("/{meow}", "/", 0)




