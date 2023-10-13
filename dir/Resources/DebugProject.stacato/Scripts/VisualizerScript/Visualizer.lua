
--visualizer script
local Visualizer = {}

local tournette0
local tournetteA
local tournetteB
local tournetteC

local tournetteAImage = of.Image()
local tournetteBImage = of.Image()
local tournetteCImage = of.Image()

local drawingMinX = -6500
local drawingMinY = -6500
local drawingMaxX = 6500
local drawingMaxY = 6500
local drawingMargin = 200.0
drawingMinX = drawingMinX - drawingMargin
drawingMinY = drawingMinY - drawingMargin
drawingMaxX = drawingMaxX + drawingMargin
drawingMaxY = drawingMaxY + drawingMargin
local drawingSizeX = drawingMaxX - drawingMinX
local drawingSizeY = drawingMaxY - drawingMinY
local aspectRatio = drawingSizeX / drawingSizeY

local largePlatformRadius = 6500
local largePlatformDiatemer = largePlatformRadius * 2
local smallPlatformRadius = 2850
local smallPlatformDiameter = smallPlatformRadius * 2
local smallPlatformDistance = 3400
local edgeLineThickness = 50


function Visualizer.setup()
	Logger:warn("Setup")
    tournetteAImage:load("tA.png")
    tournetteBImage:load("tB.png")
    tournetteCImage:load("tC.png")
    tournette0 = Environnement.getMachine("T-0"):getAnimatable("Position");
    tournetteA = Environnement.getMachine("T-A"):getAnimatable("Position");
    tournetteB = Environnement.getMachine("T-B"):getAnimatable("Position");
    tournetteC = Environnement.getMachine("T-C"):getAnimatable("Position");
end



function drawTurntable(animatable, image, name)

    local position = animatable:getActualValue()
    local brakingPosition = animatable:getBrakingPosition()

    time = of.getElapsedTimef()
    of.pushMatrix()
    of.rotateZDeg(math.sin(time*2)*30)

    of.fill()
    of.setColor(127)
    of.drawCircle(0,0,smallPlatformRadius)
    of.setColor(0)
    of.drawCircle(0,0,smallPlatformRadius-edgeLineThickness)
    of.setColor(255)
    image:draw(0, 0, smallPlatformDiameter, smallPlatformDiameter)

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

    of.setColor(16)
    of.setRectMode(of.RECTMODE_CORNER)
    of.drawRectangle(-6500, -6500, 13000, 13000)

    of.setCircleResolution(128)
    of.setRectMode(of.RECTMODE_CENTER)

    of.setColor(127)
    of.drawCircle(0,0,largePlatformRadius)
    of.setColor(32)
    of.drawCircle(0,0,largePlatformRadius-edgeLineThickness)

    of.pushMatrix()
    of.rotateZDeg(math.sin(time*10)*10)

    local TaPosition = of.Vec2f(0, smallPlatformDistance):getRotated(0)
    local TbPosition = of.Vec2f(0, smallPlatformDistance):getRotated(120)
    local TcPosition = of.Vec2f(0, smallPlatformDistance):getRotated(240)

    of.pushMatrix()
    of.translate(TaPosition.x, TaPosition.y)
    drawTurntable(tournetteA, tournetteAImage, "A")
    of.popMatrix()

    of.pushMatrix()
    of.translate(TbPosition.x, TbPosition.y)
    drawTurntable(tournetteB, tournetteBImage, "B")
    of.popMatrix()

    of.pushMatrix()
    of.translate(TcPosition.x, TcPosition.y)
    drawTurntable(tournetteC, tournetteCImage, "C")
    of.popMatrix()

    of.popMatrix() --T0 rotation matrix

    of.popMatrix() --Crop Zoom Matrix

end


--[[



function drawFlip(minX, minY, sizeX, sizeY, flipCount, flipState, bright)

    of.setRectMode(of.RECTMODE_CORNER)

    if bright then of.setColor(40) else of.setColor(100) end
    of.drawRectangle(minX, minY, sizeX, sizeY)

    local flipSizeX = sizeX / flipCount
    local actualState = flipState:getActualValue()
    local image

    if actualState == flipStates.Closed then image = flipClosedImage
    elseif actualState == flipStates.OpeningClosing then image = flipOpeningImage
    elseif actualState == flipStates.OpenLowered then image = flipOpenImage
    elseif actualState == flipStates.RaisingLowering then image = flipRaisingImage
    elseif actualState == flipStates.Raised then image = flipRaisedImage
    else image = flipOfflineImage end

    if flipState:isOffline() then of.setColor(255)
    elseif flipState:isHalted() then of.setColor(255, 255, 0, 255)
    elseif flipState:isReady() then of.setColor(255)
    else of.setColor(255, 0, 0) end

    for i=0,(flipCount-1) do
        local imageX = minX + i * flipSizeX
        image:draw(imageX, minY, flipSizeX, sizeY)
    end

end

function periacteGraphic(x, y, mirror, anchorX, anchorY, animatable)
    local sizeX = 1341.08
    local sizeY = 577.89
    x = x - anchorX * sizeX
    y = y - anchorY * sizeY
    of.pushMatrix()
    if(mirror) then
        of.scale(-1, 1)
        of.translate(-x - sizeX, y)
    else
        of.translate(x, y)
    end
    
    if animatable:isOffline() then of.setColor(0, 0, 255, 200)
    elseif animatable:isHalted() then of.setColor(255, 255, 0, 200)
    elseif animatable:isReady() then of.setColor(255, 255, 255, 200)
    else of.setColor(255, 0, 0, 200) end

    of.beginShape()
    of.vertex(72.56, 0)
    of.vertex(291.54, 471.64)
    of.vertex(1307.39, 0)
    of.endShape()

    of.setColor(0)
    of.beginShape()
    of.vertex(0, 33.69)
    of.vertex(72.56, 0)
    of.vertex(291.54, 471.64)
    of.vertex(1307.39, 0)
    of.vertex(1341.08, 72.56)
    of.vertex(252.67, 577.89)
    of.endShape()
    of.popMatrix()
end

function drawPeriactes(posY, rangeX, animatable1, animatable2, orientation1, orientation2)
    local lineThickness = 50
    of.setColor(255, 255, 255, 63)
    of.setRectMode(of.RECTMODE_CENTER)
    of.drawRectangle(0, posY, rangeX * 2, lineThickness)
    local pos1 = animatable1:getActualValue().Position * 1000
    local pos2 = animatable2:getActualValue().Position * 1000
    periacteGraphic(pos1, posY, orientation1, 1, 0.5, animatable1)
    periacteGraphic(pos2, posY, orientation2, 0, 0.5, animatable2)
end

function drawLames(posY, rangeX, animatable)
    local lineThickness = 50
    of.setColor(255, 255, 255, 63)
    of.setRectMode(of.RECTMODE_CENTER)
    of.drawRectangle(0, posY, rangeX * 2, lineThickness)

    local posX = animatable:getActualValue().Position * 1000
    local lameThickness = 100
    local rectY = posY - lameThickness * 0.5

    of.setRectMode(of.RECTMODE_CORNER)

    if animatable:isOffline() then of.setColor(0, 0, 255, 200)
    elseif animatable:isHalted() then of.setColor(255, 255, 0, 200)
    elseif animatable:isReady() then of.setColor(255, 255, 255, 200)
    else of.setColor(255, 0, 0, 200) end

    of.drawRectangle(posX, rectY - lameThickness, 3000, lameThickness * 2)
    of.drawRectangle(- 3000 - posX, rectY - lameThickness, 3000, lameThickness * 2)

    of.setColor(0)
    of.drawRectangle(posX, rectY, 1000, lameThickness)
    of.drawRectangle(posX + 2000, rectY, 1000, lameThickness)
    of.drawRectangle(- 1000 - posX, rectY, 1000, lameThickness)
    of.drawRectangle(- 3000 - posX, rectY, 1000, lameThickness)
end



function drawTournettes()
    local tournetteAnneauRotation = tournetteAnneau:getActualValue().Position
    local tournetteCentreRotation = tournetteCentre:getActualValue().Position


    of.setRectMode(of.RECTMODE_CENTER)
    of.pushMatrix()
    of.translate(0, 3563)
    of.rotateZDeg(tournetteCentreRotation)
    if tournetteCentre:isOffline() then of.setColor(100, 100, 255)
    elseif tournetteCentre:isHalted() then of.setColor(255, 255, 100)
    elseif tournetteCentre:isReady() then of.setColor(255, 255, 255)
    else of.setColor(200, 0, 0) end
    tournetteCentreImage:draw(0, 0, 7600, 7600)
    of.popMatrix()
    of.pushMatrix()
    of.translate(0, 3563)
    of.rotateZDeg(tournetteAnneauRotation)
    if tournetteAnneau:isOffline() then of.setColor(100, 100, 255)
    elseif tournetteAnneau:isHalted() then of.setColor(255, 255, 100)
    elseif tournetteAnneau:isReady() then of.setColor(255, 255, 255)
    else of.setColor(200, 0, 0) end
    tournetteAnneauImage:draw(0, 0, 7600, 7600)
    of.popMatrix()

    if Canvas.isPressed() then 
    
    of.setColor(255)
    of.setRectMode(of.RECTMODE_CENTER)
    of.pushMatrix()
    of.translate(0, 3563)
    of.rotateZDeg(tournetteCentreRotation * 50.0);
    catImage:draw(0, 0, 7000, 7000)
    of.popMatrix()

    end
end


function drawStage()

    of.setColor(64)
    of.beginShape()
    of.vertex(-14520,8519)
    of.vertex(-13020,8519)
    of.vertex(-13020,9419)
    of.vertex(-11520,9702)
    of.vertex(-10080,9940)
    of.vertex(-8640,10145)
    of.vertex(-7200,10319)
    of.vertex(-5760,10460)
    of.vertex(-4320,10570)
    of.vertex(-2880,10649)
    of.vertex(-1440,10696)
    of.vertex(0,10712)
    of.vertex(1440,10696)
    of.vertex(2880,10649)
    of.vertex(4320,10570)
    of.vertex(5760,10460)
    of.vertex(7200,10319)
    of.vertex(8640,10145)
    of.vertex(10080,9940)
    of.vertex(11520,9702)
    of.vertex(13020,9419)
    of.vertex(13020,8519)
    of.vertex(14520,8519)
    of.vertex(14520, 8562)
    --of.vertex(14520, -10438) --size of movable stuff became much smaller without costieres
   -- of.vertex(-14520, -10438)
    of.vertex(14520, -2638)
    of.vertex(-14520, -2638)
    of.vertex(-14520, 8562)
    of.endShape()

    of.setRectMode(of.RECTMODE_CENTER)
    of.setColor(0)
    of.drawRectangle(0, -2638, 29040, 50)
    of.drawRectangle(0, -2338, 29040, 50)

    --flips row G
    drawFlip(-8480, -1438, 3960, 1000, 4, Flip_GJ2, true)
    drawFlip(-4520, -1438, 3040, 1000, 3, Flip_GJ1, false)
    drawFlip(-1480, -1438, 2960, 1000, 3, Flip_Gaxe, true)
    drawFlip(1480, -1438, 3040, 1000, 3, Flip_GC1, false)
    drawFlip(4520, -1438, 3960, 1000, 4, Flip_GC2, true)

    --flips row E
    drawFlip(-9493, 1562, 1013, 1000, 1, Flip_EJ3, true)
    drawFlip(-8480, 1562, 3960, 1000, 4, Flip_EJ2, false)
    drawFlip(4520, 1562, 3960, 1000, 4, Flip_EC2, true)
    drawFlip(8480, 1562, 1013, 1000, 1, Flip_EC3, false)

    --flips row C
    drawFlip(-10506, 4562, 2026, 1000, 2, Flip_CJ3, false)
    drawFlip(-8480, 4562, 3960, 1000, 4, Flip_CJ2, true)
    drawFlip(4520, 4562, 3960, 1000, 4, Flip_CC2, false)
    drawFlip(8480, 4562, 2026, 1000, 2, Flip_CC3, true)

    --flips row A
    drawFlip(-11520, 7562, 3040, 1000, 3, Flip_AJ3, true)
    drawFlip(-8480, 7562, 3960, 1000, 4, Flip_AJ2, false)
    drawFlip(-4520, 7562, 3040, 1000, 3, Flip_AJ1, true)
    drawFlip(-1480, 7562, 2960, 1000, 3, Flip_Aaxe, false)
    drawFlip(1480, 7562, 3040, 1000, 3, Flip_AC1, true)
    drawFlip(4520, 7562, 3960, 1000, 4, Flip_AC2, false)
    drawFlip(8480, 7562, 3040, 1000, 3, Flip_AC3, true)

    --tournettes
    drawTournettes()

    --periactes
    drawPeriactes(0, 12850, periacteLointainJardin, periacteLointainCour, true, false)
    drawPeriactes(3000, 12850, periacteMilieuJardin, periacteMilieuCour, true, true)
    drawPeriactes(6002, 12850, periacteFaceJardin, periacteFaceCour, false, false)

    --lames
    drawLames(4262, 11200, lameVideoLointain)
    drawLames(4462, 11200, lameVideoFace)
end


--]]

function Visualizer.exit()
    Logger:warn("Exit")
end

return Visualizer