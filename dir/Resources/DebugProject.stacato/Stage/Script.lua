
---------------------------------------
----Default Stage Visualizer Script----
---------------------------------------


--Initialize and Load stuff here :

function setup()
	Logger:info("Starting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
	of.setCircleResolution(128)
end


--Display and Animate stuff here :

function update()
	time = of.getElapsedTimef()
	brightness = (math.sin(time) + 1.0) * 64
	diameter = (math.sin(time) + 1.0) * 100

	of.background(brightness)

	size = glm.vec2(Canvas.getSize())
	middle = glm.vec2(size) / 2.0
	of.drawCircle(middle, diameter)

	of.drawBitmapStringHighlight("Default Stage Visualizer Script", 20, 30)

	mouse = glm.vec2(Canvas.getMousePosition())
	if Canvas.isPressed() then of.setColor(255, 0, 0, 255)
	else of.setColor(0, 0, 255, 255) end
	of.drawCircle(mouse, 10)

	lStart = glm.vec2(100, 100)
	lEnd = glm.vec2(size.x - lStart.x, size.y - lStart.y)
	of.setColor(255, 0, 0, 255)
	of.drawLine(lStart, lEnd)
	lerp = (math.sin(of.getElapsedTimef() * 0.456) + 1.0) / 2.0
	rPos = glm.vec2(of.lerp(lStart.x, lEnd.x, lerp), of.lerp(lStart.y, lEnd.y, lerp))
	of.drawCircle(rPos, 50)
end


--Unload and Clean Up stuff here :

function exit()
	Logger:info("Exiting Stage Visualizer Script at", of.getElapsedTimef(), "seconds.")
end










































































































































