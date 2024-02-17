
--visualizer script
local Visualizer = {}

local tournette0
local largeFont = of.TrueTypeFont()

local drawingMinX = -6500
local drawingMinY = -6500
local drawingMaxX = 6500
local drawingMaxY = 6500
local drawingMargin = 500.0
drawingMinX = drawingMinX - drawingMargin
drawingMinY = drawingMinY - drawingMargin
drawingMaxX = drawingMaxX + drawingMargin
drawingMaxY = drawingMaxY + drawingMargin
local drawingSizeX = drawingMaxX - drawingMinX
local drawingSizeY = drawingMaxY - drawingMinY
local aspectRatio = drawingSizeX / drawingSizeY

local largePlatformRadius = 6500
local largePlatformDiatemer = largePlatformRadius * 2
local edgeLineThickness = 50


function Visualizer.setup()
	Logger:warn("Setup")
    tournette0 = Environnement.getMachine("Tournette"):getAnimatable("Position");
    largeFont:load("HelveticaBold.otf", 32)
end


function drawCenteredString(drawingFont, string, x, y)
    of.pushMatrix()
    of.scale(30)
    local textWidth = drawingFont:stringWidth(string)
    local textHeight = drawingFont:stringHeight(string)
    drawingFont:drawString(string, x-textWidth*.5, y+textHeight*.5)
    of.popMatrix()
end


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

    of.pushMatrix()
    of.scale(scaling)
    of.translate(translationX, translationY)

    time = of.getElapsedTimef()
    of.background(0)

    --begin drawing in mm coordinates here

    of.setCircleResolution(128)
    of.setRectMode(of.RECTMODE_CENTER)

    of.setColor(127)
    of.drawCircle(0,0,largePlatformRadius)
    of.setColor(32)
    of.drawCircle(0,0,largePlatformRadius-edgeLineThickness)

    local position = tournette0:getActualValue().Position
    local brakingPosition = tournette0:getBrakingPosition()
    of.pushMatrix()
    of.rotateZDeg(position)

    --draw something

    of.setColor(255)
    of.setRectMode(of.RECTMODE_CORNER)
    of.drawRectangle(-50,0,50,-largePlatformRadius)

    of.popMatrix() --T0 rotation matrix

    of.popMatrix() --Crop Zoom Matrix

end


function Visualizer.exit()
    Logger:warn("Exit")
end

return Visualizer