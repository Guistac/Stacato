
--local library = require("lib.library")
--library.increment()

---------------------------------------
----Default Environnement Script----
---------------------------------------

--Initialize stuff here :

local costiereA_Position
local costiereA_constraintA
local costiereA_constraintB

local flipStates;
local flipA_State

local flipA_constraint

function setup()

	local costiereA = Environnement.getMachine("Costière A")
	costiereA_Position = costiereA:getAnimatable("Position")

	local flipA = Environnement.getMachine("Flip A")
	flipA_State = flipA:getAnimatable("State")

	flipStates = flipA_State:getStates()

	costiereA_constraintA = costiereA_Position:createKeepoutConstraint("KeepoutA", 2.3, 4.5)
	costiereA_constraintB = costiereA_Position:createKeepoutConstraint("KeepoutB", 6.7, 8.0)

	flipA_constraint = flipA_State:createHaltConstraint("Avoid Costiere A")

end


--Perform Environnement Logic here :


function update()
	local flipAState = flipA_State:getActualValue()
	local flipA_Blocking = flipAState ~= flipStates.Closed
	costiereA_constraintA:setEnabled(flipA_Blocking)

	local costiereA_now = costiereA_Position:getActualValue().Position
	local costiereA_brk = costiereA_Position:getBrakingPosition()
	local costiereA_BlockFlipA = checkRangeOverlap(costiereA_now, costiereA_brk, 2.3, 4.5)
	costiereA_constraintB:setEnabled(costiereA_BlockFlipA)

end


--Clean Up stuff here :

function exit()
end
























































