--Initialize and Load stuff here :

local costiereA = Environnement.getMachine("Costière A")
local costiereA_Position = costiereA:getAnimatable("Position")

local flipB3 = Environnement.getMachine("Flip B3")
local flipB3_State = flipB3:getAnimatable("State")

local flipA4 = Environnement.getMachine("Flip A4")
local flipA4_State = flipA4:getAnimatable("State")

local flipStates = flipB3_State:getStates()



function setup()
end




function update()

	local costiereA_Position = costiereA_Position:getActualValue()
	local flipB3_State = flipB3_State:getActualValue()
	local flipA4_State = flipA4_State:getActualValue()

	local canvasSize = glm.vec2(Canvas.getSize())

	--draw background with acceleration
	local brightness = math.abs(costiereA_Position.Acceleration * 16)
	of.background(brightness)

	--draw circle with velocity
	local diameter = (costiereA_Position.Velocity / 1) * 100
	local middle = glm.vec2(canvasSize.x / 2.0, canvasSize.y / 2.0)
	of.drawCircle(middle, diameter)

	--draw mouse cursor
	local mouse = glm.vec2(Canvas.getMousePosition())
	if Canvas.isPressed() then of.setColor(255, 0, 0, 255)
	else of.setColor(0, 0, 255, 255) end
	of.drawCircle(mouse, 10)

	--draw line and moving circle with position
	local lStart = glm.vec2(100, 100)
	local lEnd = glm.vec2(canvasSize.x - lStart.x, canvasSize.y - lStart.y)
	of.setColor(255, 0, 0, 255)
	of.drawLine(lStart, lEnd)
	local lerp = costiereA_Position.Position / 10
	local rPos = glm.vec2(of.lerp(lStart.x, lEnd.x, lerp), of.lerp(lStart.y, lEnd.y, lerp))
	of.drawCircle(rPos, 50)

	local B3stateString = "Flip B3: " .. flipB3_State:toString()
	local A4stateString = "Flip A4: " .. flipA4_State:toString()
	of.drawBitmapStringHighlight(B3stateString, 20, canvasSize.y - 50)
	of.drawBitmapStringHighlight(A4stateString, 20, canvasSize.y - 30)

end


--Unload and Clean Up stuff here :

function exit()
	Logger:info("Exiting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
end
























































































