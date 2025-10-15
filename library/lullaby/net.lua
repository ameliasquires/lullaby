local common = require("common")
---@meta

---@class net
---@field mimetypes string filepath for mimetype definitions (defaults to /etc/mime.types). nil to skip
local net = {}

---@class server-table
local server_table = {}

---@class res-table
local res_table = {}

---sends value to client and closes socket
---@param T res-table
---@param value string
function res_table.send(T, value) end

---autosets Content-Type and sends contents of file to client and closes socket
---@param T res-table
---@param value string
---@return error | nil error
function res_table.sendfile(T, value) end

---sends value to client and doesn't close the socket
---@param T res-table
---@param value string
function res_table.write(T, value) end

---closes socket
---@param T res-table
function res_table.close(T) end

---prevents calling any other selected routes
---@param T res-table
function res_table.stop(T) end

---key value table containing header values to be sent
res_table.header = {}

---@class req-table
local req_table = {}

---"roll" the request forward
---@param T req-table
---@param bytes integer | nil
function req_table.roll(T, bytes) end

---list of parameters in route 
req_table.parameters = {}

---@deprecated
req_table.client_fd = 0

---@deprecated
req_table._bytes = 0

---@type integer
req_table.ip = 0

---@type string
req_table.Body = ""

---@type string
req_table.path = ""

---@type string
req_table.rawpath = ""

---@type any|nil
req_table.files = {}

---@type any|nil
req_table.cookies = {}

---@type any|nil
req_table.query = {}

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.GET(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.HEAD(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.POST(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.PUT(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.DELETE(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.CONNECT(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.OPTIONS(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.TRACE(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.PATCH(T, route, callback) end

---@param T server-table
---@param route string
---@param callback fun(res: res-table, req: req-table)
function server_table.all(T, route, callback) end

---sends a signal to stop accepting requests, server will shutdown, but already accepted ones will still continue
---@param T server-table
function server_table.close(T) end

---@param server server-table
local function listen_callback(server) end

---@param callback fun(server: server-table)
---@param port integer
function net.listen(callback, port) end

---@class request-return
---@field content stream
---@field code integer response code
---@field code-name string response message
---@field version string http version

---creates an https request
---@param url string
---@param content string | nil
---@param header table<string, string> | nil
---@param request string | nil
---@return request-return | error
function net.srequest(url, content, header, request) end

---creates an http request
---@param url string
---@param content string | nil
---@param header table<string, string> | nil
---@param request string | nil
---@return request-return | error
function net.request(url, content, header, request) end

---@class wss-table
local wss = {}

---@class wss-read
local wss_read = {}

---@type string
wss_read.content = ""

---@type integer
wss_read.opcode = 0

---reads oldest unread frame from server
---@param T wss-table
---@return wss-read | error
function wss.read(T) end

---sents data frame to server
---@param T wss-table
---@param data string
---@return nil | error
function wss.write(T, data) end

---calls gc early
---@param T wss-table
---@return nil
function wss.close(T) end

---creates a wss connection
---@param url string
---@return wss-table | error
function net.wss(url) end

return net
