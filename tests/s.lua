require "llib"
function _G.sleep (a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end

for i=1,50 do
    llib.net.spawn(function()
        local sec = tonumber(os.clock() + 1); 
    while (os.clock() < sec) do 
    end 
        print("hi")
    end)
end

while true do end