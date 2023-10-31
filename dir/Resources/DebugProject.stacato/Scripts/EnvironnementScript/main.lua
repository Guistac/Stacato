
---------------------------------------
----Default Environnement Script----
---------------------------------------

local p1Jardin
local p1Cour
local p2Jardin
local p2Cour
local p3Jardin
local p3Cour

local p1J_Lim
local p1C_Lim
local p2J_Lim
local p2C_Lim
local p3J_Lim
local p3C_Lim

--Initialize stuff here :

function setup()

	p1Jardin = Environnement.getMachine("Périacte Lointain Jardin"):getAnimatable("Position")
	p1Cour = Environnement.getMachine("Périacte Lointain Cour"):getAnimatable("Position")
	p2Jardin = Environnement.getMachine("Périacte Milieu Jardin"):getAnimatable("Position")
	p2Cour = Environnement.getMachine("Périacte Milieu Cour"):getAnimatable("Position")
	p3Jardin = Environnement.getMachine("Périacte Face Jardin"):getAnimatable("Position")
	p3Cour = Environnement.getMachine("Périacte Face Cour"):getAnimatable("Position")

	p1J_Lim = p1Jardin:createKeepoutConstraint("Anticollision Périacte Lointain Cour", 0.0, 0.0)
	p1C_Lim = p1Cour:createKeepoutConstraint("Anticollision Périacte Lointain Jardin", 0.0, 0.0)
	p2J_Lim = p2Jardin:createKeepoutConstraint("Anticollision Périacte Milieu Cour", 0.0, 0.0)
	p2C_Lim = p2Cour:createKeepoutConstraint("Anticollision Périacte Milieu Jardin", 0.0, 0.0)
	p3J_Lim = p3Jardin:createKeepoutConstraint("Anticollision Périacte Face Cour", 0.0, 0.0)
	p3C_Lim = p3Cour:createKeepoutConstraint("Anticollision Périacte Face Jardin", 0.0, 0.0)

end


--Perform Environnement Logic here :

function update()

	local MinDistance = 1.0

	local p1j_MaxPosition = math.min(p1Cour:getBrakingPosition(), p1Cour:getActualValue().Position) - MinDistance
	local p1c_MinPosition = math.max(p1Jardin:getBrakingPosition(), p1Jardin:getActualValue().Position) + MinDistance
	p1J_Lim:adjust(p1j_MaxPosition, 999)
	p1C_Lim:adjust(-999, p1c_MinPosition)

	local p2j_MaxPosition = math.min(p2Cour:getBrakingPosition(), p2Cour:getActualValue().Position) - MinDistance
	local p2c_MinPosition = math.max(p2Jardin:getBrakingPosition(), p2Jardin:getActualValue().Position) + MinDistance
	p2J_Lim:adjust(p2j_MaxPosition, 999)
	p2C_Lim:adjust(-999, p2c_MinPosition)

	local p3j_MaxPosition = math.min(p3Cour:getBrakingPosition(), p3Cour:getActualValue().Position) - MinDistance
	local p3c_MinPosition = math.max(p3Jardin:getBrakingPosition(), p3Jardin:getActualValue().Position) + MinDistance
	p3J_Lim:adjust(p3j_MaxPosition, 999)
	p3C_Lim:adjust(-999, p3c_MinPosition)


end


--Clean Up stuff here :

function exit()
end

























