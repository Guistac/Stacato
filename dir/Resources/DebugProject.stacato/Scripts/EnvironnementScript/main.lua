
local upperBrakeState
local brakeStateClosed
local brakeStateOpen

local bookAxisPosition

local upperBrakeHaltConstraint
local bookAxisConstraint

local bookAxisLimit = 15
local infinity = 999999999

function setup()
	local upperBrake = Environnement.getMachine("Fr. Plateforme")
	upperBrakeState = upperBrake:getAnimatable("State")

	local brakeStates = upperBrakeState:getStates()
	brakeStateOpen = brakeStates["Open"]
	brakeStateClosed = brakeStates["Closed"]

	bookAxisPosition = Environnement.getMachine("Levage Mur"):getAnimatable("Position")
	
	upperBrakeHaltConstraint = upperBrakeState:createHaltConstraint("Interdiction Ouverture Frein en Levage")
	bookAxisConstraint = bookAxisPosition:createKeepoutConstraint("Interdiction Levage avec frein ouvert", bookAxisLimit, infinity)
end

function update()


	local bookAxisActualPos = bookAxisPosition:getActualValue().Position
	local upperBrakeActualState = upperBrakeState:getActualValue()

	--constraint the book axis if the brake is not closed
	bookAxisConstraint:setEnabled(upperBrakeActualState ~= brakeStateClosed)

	--constraint the brake if the book axis is too high
	upperBrakeHaltConstraint:setEnabled(bookAxisActualPos > bookAxisLimit + .5)

end

function exit()
end





























































































































































































