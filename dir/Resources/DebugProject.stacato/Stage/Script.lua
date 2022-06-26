
---------------------------------------
----Default Stage Visualizer Script----
---------------------------------------


--Initialize and Load stuff here :

local costiereA = Environnement.getMachine("Costière A")
local costiereA_Position = costiereA:getAnimatable("Position")

function setup()
	Logger:info("Starting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
	of.setCircleResolution(128)
end


--Display and Animate stuff here :

function update()

	local actualPosition = costiereA_Position:getPosition()
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



















































































































































































































































