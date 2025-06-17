## request

net.request(url, content, header, request)**
net.srequest(url, content, header, request)

both function identically, where srequest uses https instead of http

content is a string with the body of the request

header is just a table

request is just the request method, ie: GET, PUT, ...

will call each other when the url protocol mismatches the function

can return an error

```lua
local response = net.srequest("https://amyy.cc")
```

the response is a table consisting of the response headers with a few extra values

|name|description|
| -- | --        |
| code | response code |
| code-name | response code string |
| version | http version |
| content | a stream containing the body of the request |
