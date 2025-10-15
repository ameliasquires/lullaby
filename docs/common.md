## errors

errors will typically be created and propogated using luaI_error (in c) but will always retain a common style (unless mentioned otherwise)

it will return 3 values, in order

* nil (just always a nil value first, useful to do a quick check for errors on functions with a return value)
* string (an error message)
* integer (an error code)

similarily, when luaI_assert is called, the string will be the expression and the integer will be -1

## stream

this is a generic function used in many places.

stream:read(?bytes)
stream:file(filename, ?bytes, ?mode)

bytes is an optional value allowing you to select how many bytes at maximum to read. this value can be ignored or adjusted by the function, and if so, it will be noted in the docs (default is nil, and will read as much as possible)

mode is the mode the file will be opened with (defaults to "w")
