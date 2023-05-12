
--Initialize stuff here :

local rotCorbeau
local transCorbeau

local upperTransConstraint
local lowerTransConstraint
local rotConstraint


function setup()

local machineRotation = Environnement.getMachine("Rot. Corbeau")
local machineTranslation = Environnement.getMachine("Cost. Corbeau")
rotCorbeau = machineRotation:getAnimatable("Position")
transCorbeau = machineTranslation:getAnimatable("Position")

upperTransConstraint = transCorbeau:createKeepoutConstraint("Upper Anticollision Translation", 9.0, 99)
lowerTransConstraint = transCorbeau:createKeepoutConstraint("Lower Anticollision Translation", 0.0, 8.5)
rotConstraint = rotCorbeau:createKeepoutConstraint("Anticollision Rotation", -40, -140)

end


--Perform Environnement Logic here :

function update()

	local translationPos = transCorbeau:getActualValue().Position
	local translationBrakePos = transCorbeau:getBrakingPosition()
	local isBelowRotPos = checkRangeOverlap(translationPos, translationBrakePos, 0.0, 8.5 + 0.05)
	local isAboveRotPos = checkRangeOverlap(translationPos, translationBrakePos, 9.0 - 0.05, 99)
	local transIsOffline = transCorbeau:isOffline()
	local enableRotationConstraint = transIsOffline or isBelowRotPos or isAboveRotPos
	rotConstraint:setEnabled(enableRotationConstraint)

	local rotationPos = rotCorbeau:getActualValue().Position
	local rotationBrakePos = rotCorbeau:getBrakingPosition()
	local rotIsBlockingTrans = checkRangeOverlap(rotationPos, rotationBrakePos, -40 - 0.05, -140 + 0.05)
	local rotIsOffline = rotCorbeau:isOffline()
	local enableTranslationConstraint = rotIsOffline or rotIsBlockingTrans
	upperTransConstraint:setEnabled(enableTranslationConstraint)
	lowerTransConstraint:setEnabled(enableTranslationConstraint)


end


--Clean Up stuff here :

function exit()
end





































