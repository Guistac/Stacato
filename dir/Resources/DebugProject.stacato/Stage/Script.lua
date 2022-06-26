
---------------------------------------
----Default Stage Visualizer Script----
---------------------------------------


--Initialize and Load stuff here :

local costiereA = Environnement.getMachine("Costière A")
local costiereA_Position = costiereA:getAnimatable("Position")

local flipB3 = Environnement.getMachine("Flip B3")
local flipB3_State = flipB3:getAnimatable("State")

local baseclasses = flipB3_State.BaseClasses


Logger:warn("---------", baseclasses)
for key, value in pairs(baseclasses) do
	Logger:warn("base class", key, value)
end
Logger:warn("---------")

local name = flipB3_State:getName()

local flipA4 = Environnement.getMachine("Flip A4")
local flipA4_State = flipA4:getAnimatable("State")

local flipStates = flipB3_State:getStates()

function setup()
	Logger:info("Starting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
	of.setCircleResolution(128)

	for name, state in pairs(flipStates) do
		Logger:info("state", name, "=", state:toInteger())
	end

end


--Display and Animate stuff here :

function update()

	local actualPosition = costiereA_Position:getActualValue():toPosition()
	local pos = actualPosition.Position
	local vel = actualPosition.Velocity
	local acc = actualPosition.Acceleration

	local time = of.getElapsedTimef()
	local brightness = math.abs(acc * 16)
	local diameter = (vel / 1) * 100

	of.background(brightness)

	local size = glm.vec2(Canvas.getSize())
	local middle = glm.vec2(size.x / 2.0, size.y / 2.0)
	of.drawCircle(middle, diameter)

	of.drawBitmapStringHighlight("Default Stage Visualizer Script", 20, 30)

	local mouse = glm.vec2(Canvas.getMousePosition())
	if Canvas.isPressed() then of.setColor(255, 0, 0, 255)
	else of.setColor(0, 0, 255, 255) end
	of.drawCircle(mouse, 10)

	local lStart = glm.vec2(100, 100)
	local lEnd = glm.vec2(size.x - lStart.x, size.y - lStart.y)
	of.setColor(255, 0, 0, 255)
	of.drawLine(lStart, lEnd)
	local lerp = pos / 10
	local rPos = glm.vec2(of.lerp(lStart.x, lEnd.x, lerp), of.lerp(lStart.y, lEnd.y, lerp))
	of.drawCircle(rPos, 50)

end


--Unload and Clean Up stuff here :

function exit()
	Logger:info("Exiting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
end

