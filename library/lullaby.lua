---@meta

---@class lullaby
local lullaby = {}

function lullaby.version() end

lullaby.crypto = require("library.lullaby.crypto")
lullaby.error = require("library.lullaby.error")
lullaby.io = require("library.lullaby.io")
lullaby.math = require("library.lullaby.math")
lullaby.net = require("library.lullaby.net")
lullaby.sort = require("library.lullaby.sort")
lullaby.table = require("library.lullaby.table")
lullaby.test = require("library.lullaby.test")
lullaby.thread = require("library.lullaby.thread")

return lullaby
