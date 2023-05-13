
--Initialize stuff here :

local rotCorbeau
local transCorbeau

local transConstraint
local rotConstraint

local rotationToTranslationLimit = {
	{-130, 8.5},
	{-125, 8.45},
	{-120, 8.4},
	{-115, 8.35},
	{-110, 8.3},
	{-105, 8.2},
	{-100, 8.1},
	{-95,  7.95},
	{-90,  7.85},
	{-85,  7.7},
	{-80,  7.6},
	{-75,  7.4},
	{-70,  7.25},
	{-65,  7.1},
	{-60,  6.95},
	{-55,  6.75},
	{-50,  6.55},
	{-46,  6.55},
	{-45,  4.5},
	{-5,   3.5},
	{-4.9, 0.0}
}

local translationToRotationLimit = {
	{0.0,  -4.9},
	{3.5,  -5},
	{4.5,  -45},
	{6.55, -46},
	{6.55, -50},
	{6.75, -55},
	{6.95, -60},
	{7.1,  -65},
	{7.25, -70},
	{7.4,  -75},
	{7.6,  -80},
	{7.7,  -85},
	{7.85, -90},
	{7.95, -95},
	{8.1,  -100},
	{8.2,  -105},
	{8.3,  -110},
	{8.35, -115},
	{8.4,  -120},
	{8.45, -125},
	{8.5,  -185}
}

--look up table function
--the inputTable has to be ordered ascending
--the outputTable has to have the same amount of elements as the input table
--the function returns a remapped value and a boolean to indicate if the input value is in range of the inputTable
function lookupTable(input, lookupPoints)
	local pointCount = #lookupPoints
	for i=1,pointCount do
		if(input < lookupPoints[i][1]) then
			if(i == 1) then
				return lookupPoints[i][2], false
			else
				local normalized = (input - lookupPoints[i-1][1]) / (lookupPoints[i][1] - lookupPoints[i-1][1])
				local output = (lookupPoints[i][2] - lookupPoints[i-1][2]) * normalized + lookupPoints[i-1][2]
				return output, true
			end
		end
	end
	if(input == lookupPoints[pointCount][1]) then return lookupPoints[pointCount][2], true
	else return lookupPoints[pointCount][2],false end
end


function setup()

	local machineRotation = Environnement.getMachine("Rot. Corbeau")
	local machineTranslation = Environnement.getMachine("Cost. Corbeau")
	rotCorbeau = machineRotation:getAnimatable("Position")
	transCorbeau = machineTranslation:getAnimatable("Position")
	
	transConstraint = transCorbeau:createKeepoutConstraint("Anticollision Translation", 9.0, 99)
	rotConstraint = rotCorbeau:createKeepoutConstraint("Anticollision Rotation", -40, -140)
	
	rotConstraint:setEnabled(true)
	transConstraint:setEnabled(true)

end


--Perform Environnement Logic here :

function update()

	local translationBrakePos = transCorbeau:getBrakingPosition()
	local rotationLimit = lookupTable(translationBrakePos, translationToRotationLimit)
	rotConstraint:adjust(-300, rotationLimit)

	local rotationBrakePos = rotCorbeau:getBrakingPosition()
	local upperTranslationLimit = lookupTable(rotationBrakePos, rotationToTranslationLimit)
	transConstraint:adjust(0.0, upperTranslationLimit)

end


--Clean Up stuff here :

function exit()
end









