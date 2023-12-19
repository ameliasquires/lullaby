# table (tables and sorting)

## sorting

|name|accepted types|type of sorting|order|
|---|---|---|---|
|quicksort| double,int | comparison | greatest -> least |
|mergesort| double,int | comparison | greatest -> least |
|shellsort| double,int | comparison | greatest -> least |
|bubblesort| double,int | comparison | greatest -> least |
|heapsort| double,int | comparison | greatest -> least |
|countingsort| int | non-comparison | least -> greatest|
|miraclesort| double,int | esoteric | ? greatest -> least |
|stalinsort| double,int | esoteric | greatest -> least |
|slowsort| double,int | esoteric | greatest -> least |
|bogosort| double,int | esoteric | greatest -> least |

### usage

```lua
local test = {5, 10, 922, -32, 554, 0};
locla test_sorted = llib.array.mergesort(test); -- 922, 554, 10, 5, 0, -32
```

## utils

### len 

'accepts any table 

returns length of table, including associative arrays

```lua
llib.array.len({a = 55, b = 22}); -- 2
```

### reverse 

'accepts any table 

reverses a table

```lua
llib.array.reverse({5, 4, 22}) -- 22, 4, 5
```

### least/greatest 

'accepts a table with numbers 

gets smallest/largest value of a table

```lua
llib.array.greatest({22, 99, 99.9}) -- 99.9
```

### shuffle 

'accepts any table

randomizes array 

```lua
llib.array.shuffle({5, 22, 89}) -- 22, 89, 5
```
### sum 

'accepts a table with numbers 

gets sum of each element in a table 

```lua
llib.array.sum({5, 4, 3}) --12
```

### index

'accepts any table and a value

gets index of item in array linearly, returns -1 if not present

```lua
llib.array.index({5, 4, 3}, 4) -- 2
```

### sindex 

'accepts a sorted table with numbers and a value

gets index through binary search, -1 if not present

```lua
llib.array.sindex({2, 4, 8}, 8) -- 3
```

### split 

'accepts a string and a delimiter

splits a string by arg2, returns a array 

```lua
llib.array.split("hello world"," ") -- "hello", "world"
```

### to_char_array 

'accepts a string 

returns a char array from a string 

```lua
llib.array.to_char_array("meow") -- "m", "e", "o", "w"
```

