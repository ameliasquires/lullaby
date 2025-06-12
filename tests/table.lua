local llby = require"lullaby"
llby.io.pprint(llby.table.split("/hello/world//test///", "/", false)) -- {"hello", "world", "", "test", "", "", ""}

llby.io.pprint(llby.table.split("/hi/uwu/meow", "/", false))
