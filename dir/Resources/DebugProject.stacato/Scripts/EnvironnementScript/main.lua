
---------------------------------------
----Default Environnement Script----
---------------------------------------


--Initialize stuff here :

local testMachine
local refSignal


function setup()

testMachine = Environnement.getMachine("TestMachine")
refSignal = testMachine:getNodePin("Reference Signal")
local value = refSignal:getBoolValue()
Logger:warn(value)

end


--Perform Environnement Logic here :

function update()
end


--Clean Up stuff here :

function exit()
end























