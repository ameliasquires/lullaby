require "llib"
function sleep (a) 
    local sec = tonumber(os.clock() + a); 
    while (os.clock() < sec) do 
    end 
end

for i=1,500 do
    llib.net.spawn(function()
        --sleep(1)
        print("hi")
    end)
end

while true do end