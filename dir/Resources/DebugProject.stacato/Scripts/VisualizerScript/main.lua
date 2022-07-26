
---------------------------------------
----Default Stage Visualizer Script----
---------------------------------------

--Logger:warn(package.path)

--local library = require("lib.library")

--Logger:warn(library.get())
--library.increment()
--Logger:warn(library.get())

--local image = of.Image()

--Initialize and Load stuff here :

local costiereA = Environnement.getMachine("Costière A")
local costiereAPosition = costiereA:getAnimatable("Position")
local costiereAPositionConstraints = costiereAPosition:getConstraints()

local flipA = Environnement.getMachine("Flip A")
local flipAState = flipA:getAnimatable("State")
local flipStates = flipAState:getStates()

function setup()
	Logger:info("Starting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
	of.setCircleResolution(128)
	--image:load("test.jpg")
	--Logger:warn(image:getWidth(), image:getHeight())

	for k, v in pairs(flipStates) do
		Logger:warn(k,v)
	end

end


--Display and Animate stuff here :

function update()

	local min = 0.1
	local max = 9.9
	local act = (costiereAPosition:getActualValue().Position - min) / (max - min)
	local brk = (costiereAPosition:getBrakingPosition() - min) / (max - min)

	

	--time = of.getElapsedTimef()
	--brightness = (math.sin(time) + 1.0) * 64
	--diameter = (math.sin(time) + 1.0) * 100

	of.background(0, 0, 0)

	size = glm.vec2(Canvas.getSize())
	--middle = glm.vec2(size.x / 2.0, size.y / 2.0)
	--of.drawCircle(middle, diameter)

	of.drawBitmapStringHighlight("Default Stage Visualizer Script", 20, 30)

	--mouse = glm.vec2(Canvas.getMousePosition())
	--if Canvas.isPressed() then of.setColor(255, 0, 0, 255)
	--else of.setColor(0, 0, 255, 255) end
	--of.drawCircle(mouse, 10)

	lStart = glm.vec2(100, 100)
	lEnd = glm.vec2(size.x - lStart.x, size.y - lStart.y)
	of.setColor(255, 0, 0, 255)
	of.drawLine(lStart, lEnd)

	for i, constraint in ipairs(costiereAPositionConstraints) do
		from = (constraint:getMin() - min) / (max - min)
		to = (constraint:getMax() - min) / (max - min)
		
	end

	actPos = glm.vec2(of.lerp(lStart.x, lEnd.x, act), of.lerp(lStart.y, lEnd.y, act))
	brkPos = glm.vec2(of.lerp(lStart.x, lEnd.x, brk), of.lerp(lStart.y, lEnd.y, brk))
	of.fill();
	of.setColor(64)
	of.drawCircle(brkPos, 50)
	of.setColor(255, 0, 0, 255)
	of.drawCircle(actPos, 50)

	--of.setRectMode(of.RECTMODE_CENTER)
	--of.setColor(255)
	--image:draw(actPos, image:getWidth() / 10, image:getHeight() / 10)

	local flipAValue = flipAState:getActualValue()

	of.drawBitmapStringHighlight(flipAValue:toString(), 20, 50)

end


--Unload and Clean Up stuff here :

function exit()
	Logger:info("Exiting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
end






























































