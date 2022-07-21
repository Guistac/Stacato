
--Logger:warn(package.path)

--local library = require("lib.library")

--Logger:warn(library.get())
--library.increment()
--Logger:warn(library.get())

--ModuleTest.increment()

---------------------------------------
----Default Environnement Script----
---------------------------------------

--Initialize stuff here :

local costiereA_Position
local costiereA_constraintA
local costiereA_constraintB

local flipStates;
local flipA_State

function setup()

	local costiereA = Environnement.getMachine("Costière A")
	costiereA_Position = costiereA:getAnimatable("Position")

	local flipA = Environnement.getMachine("Flip A")
	--flipA_State = flipA:getAnimatable("State")
	--flipStates = flipA_State:getStates()

	--Logger:warn("logging flip states")
	--for k, v in pairs(flipStates) do
	--	Logger:warn(k, v)
	--end

	costiereA_constraintA = costiereA_Position:createKeepoutConstraint("KeepoutA", 2.3, 4.5)
	costiereA_constraintB = costiereA_Position:createKeepoutConstraint("KeepoutB", 6.7, 8.0)

end


--Perform Environnement Logic here :


function update()

	--local flipA_ActualState = flipA_State:getActualValue()
	--if flipA_ActualState == flipStates.Open then
	--end

end


--Clean Up stuff here :

function exit()
end
























