
local flipStates;

local flipGC2_State
local flipGC2_HaltConstraint

local flipEC2_State
local flipEC2_HaltConstraint

local flipCC2_State
local flipCC2_HaltConstraint

local costiereCour_Position
local costiereCour_constraintGC2
local costiereCour_constraintEC2
local costiereCour_constraintCC2

function setup()

	--- FLIPS setup

	local flipGC2 = Environnement.getMachine("Flip G-C2")
	local flipEC2 = Environnement.getMachine("Flip E-C2")
	local flipCC2 = Environnement.getMachine("Flip C-C2")

	flipGC2_State = flipGC2:getAnimatable("State")
	flipEC2_State = flipEC2:getAnimatable("State")
	flipCC2_State = flipCC2:getAnimatable("State")

	flipStates = flipGC2_State:getStates()

	flipGC2_HaltConstraint = flipGC2_State:createHaltConstraint("Anti-Collision Costière Cour")
	flipEC2_HaltConstraint = flipEC2_State:createHaltConstraint("Anti-Collision Costière Cour")
	flipCC2_HaltConstraint = flipCC2_State:createHaltConstraint("Anti-Collision Costière Cour")

	--- COSTIERES setup

	local costiereCour = Environnement.getMachine("Cost Cour")
	costiereCour_Position = costiereCour:getAnimatable("Position")

	costiereCour_constraintGC2 = costiereCour_Position:createKeepoutConstraint("Keepout G-C2", 4.20, 9.36)
	costiereCour_constraintEC2 = costiereCour_Position:createKeepoutConstraint("Keepout E-C2", 7.20, 99.9)
	costiereCour_constraintCC2 = costiereCour_Position:createKeepoutConstraint("Keepout C-C2", 10.20, 99.9)
end


function update()

	------- Costiere Cour // Flipts GC2 EC2 CC2
	local GC2_blocking = flipGC2_State:getActualValue() ~= flipStates.Closed or not flipGC2_State:isOnline()
	costiereCour_constraintGC2:setEnabled(GC2_blocking)

	local EC2_blocking = flipEC2_State:getActualValue() ~= flipStates.Closed or not flipEC2_State:isOnline()
	costiereCour_constraintEC2:setEnabled(EC2_blocking)

	local CC2_blocking = flipCC2_State:getActualValue() ~= flipStates.Closed or not flipCC2_State:isOnline()
	costiereCour_constraintCC2:setEnabled(CC2_blocking)

	local costCourActPos = costiereCour_Position:getActualValue().Position
	local costCourBrkPos = costiereCour_Position:getBrakingPosition()

	local GC2_blocked = checkRangeOverlap(costCourActPos, costCourBrkPos, 4.22, 9.34) or not costiereCour_Position:isOnline()
	flipGC2_HaltConstraint:setEnabled(GC2_blocked)

	local EC2_blocked = checkRangeOverlap(costCourActPos, costCourBrkPos, 7.22, 99.9) or not costiereCour_Position:isOnline()
	flipEC2_HaltConstraint:setEnabled(EC2_blocked)

	local CC2_blocked = checkRangeOverlap(costCourActPos, costCourBrkPos, 10.22, 99.9) or not costiereCour_Position:isOnline()
	flipCC2_HaltConstraint:setEnabled(CC2_blocked)

end


--Clean Up stuff here :

function exit()
end











