
--visualizer script
local Visualizer = {}

function Visualizer.setup()
	Logger:warn("Setup")
end



local drawingMinX = -11520
local drawingMinY = -8562.5
local drawingMaxX = 11520
local drawingMaxY = 8562.5
local drawingMargin = 500.0

drawingMinX = drawingMinX - drawingMargin
drawingMinY = drawingMinY - drawingMargin
drawingMaxX = drawingMaxX + drawingMargin
drawingMaxY = drawingMaxY + drawingMargin
local drawingSizeX = drawingMaxX - drawingMinX
local drawingSizeY = drawingMaxY - drawingMinY
local aspectRatio = drawingSizeX / drawingSizeY

function Visualizer.draw()
    local canvasWidth, canvasHeight = Canvas.getSize()
    local canvasAspectRatio = canvasWidth / canvasHeight
    local scaling, translationX, translationY
    if aspectRatio < canvasAspectRatio then
        scaling = canvasHeight / drawingSizeY
        local offsetX = (canvasWidth - drawingSizeX * scaling) / 2.0
        translationX = -drawingMinX + offsetX / scaling
        translationY = -drawingMinY
    else
        scaling = canvasWidth / drawingSizeX
        local offsetY = (canvasHeight - drawingSizeY * scaling) / 2.0
        translationX = -drawingMinX
        translationY = -drawingMinY + offsetY / scaling
    end

    of.background(0)
    of.pushMatrix()
    of.scale(scaling)
    of.translate(translationX, translationY)

    drawStage()

    of.popMatrix()

end


function drawStage()

    of.setColor(0,0,0,64)
    of.drawRectangle(drawingMinX, drawingMinY, drawingSizeX, drawingSizeY)

    --flips row 1
    of.setColor(255)
    of.drawRectangle(-8480, -1438, 3960, 1000)
    of.setColor(200)
    of.drawRectangle(-4520, -1438, 3040, 1000)
    of.setColor(255)
    of.drawRectangle(-1480, -1438, 2960, 1000)
    of.setColor(200)
    of.drawRectangle(1480, -1438, 3040, 1000)
    of.setColor(255)
    of.drawRectangle(4520, -1438, 3960, 1000)

    --flips row 2
    of.setColor(200)
    of.drawRectangle(-11520, 1562, 3040, 1000)
    of.setColor(255)
    of.drawRectangle(-8480, 1562, 3960, 1000)
    of.setColor(255)
    of.drawRectangle(4520, 1562, 3960, 1000)
    of.setColor(200)
    of.drawRectangle(8480, 1562, 3040, 1000)

    --flips row 3
    of.setColor(200)
    of.drawRectangle(-11520, 4562, 3040, 1000)
    of.setColor(255)
    of.drawRectangle(-8480, 4562, 3960, 1000)
    of.setColor(255)
    of.drawRectangle(4520, 4562, 3960, 1000)
    of.setColor(200)
    of.drawRectangle(8480, 4562, 3040, 1000)

    --flips row 4
    of.setColor(200)
    of.drawRectangle(-11520, 7562, 3040, 1000)
    of.setColor(255)
    of.drawRectangle(-8480, 7562, 3960, 1000)
    of.setColor(200)
    of.drawRectangle(-4520, 7562, 3040, 1000)
    of.setColor(255)
    of.drawRectangle(-1480, 7562, 2960, 1000)
    of.setColor(200)
    of.drawRectangle(1480, 7562, 3040, 1000)
    of.setColor(255)
    of.drawRectangle(4520, 7562, 3960, 1000)
    of.setColor(200)
    of.drawRectangle(8480, 7562, 3040, 1000)

    --turntables
    of.setRectMode(of.RECTMODE_CENTER)
    of.setCircleResolution(64)
    of.setColor(200)
    of.drawCircle(0, 3563, 3800) -- center & radius
    of.setColor(255)
    of.drawCircle(0, 3563, 2800) -- center & radius


    --costieres
    of.setColor(255)
    of.drawLine(-6510, -8562, -6510, 7562)
    of.drawLine(6510, -8562, 6510, 7562)


    --center cross hair
    --[[
    of.setColor(255,0,0)
    of.drawRectangle(-100,-100,100,100)
    of.drawRectangle(0,0,100,100)
    of.setColor(0,0,255)
    of.drawRectangle(0,-100,100,100)
    of.drawRectangle(-100,0,100,100)
    ]]--
end


function Visualizer.exit()
    Logger:warn("Exit")
end

return Visualizer