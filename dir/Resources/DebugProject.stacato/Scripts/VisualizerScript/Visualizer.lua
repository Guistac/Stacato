
--visualizer script
local Visualizer = {}


local lameVideoFace
local lameVideoLointain
local periacteLointainJardin
local periacteLointainCour
local periacteMilieuJardin
local periacteMilieuCour
local periacteFaceJardin
local periacteFaceCour

local Flip_GJ2
local Flip_GJ1
local Flip_Gaxe
local Flip_GC1
local Flip_GC2

local Flip_EJ3
local Flip_EJ2
local Flip_EC2
local Flip_EC3

local Flip_CJ3
local Flip_CJ2
local Flip_CC2
local Flip_CC3

local Flip_AJ3
local Flip_AJ2
local Flip_AJ1
local Flip_Aaxe
local Flip_AC1
local Flip_AC2
local Flip_AC3

local tournetteAnneau
local tournetteCentre

local costiereJardin
local costiereCour

local volJardin
local volCour

local romaine

local flipStates
local romaineStates

local flipOfflineImage = of.Image()
local flipClosedImage = of.Image()
local flipOpeningImage = of.Image()
local flipOpenImage = of.Image()
local flipRaisingImage = of.Image()
local flipRaisedImage = of.Image()
local tournetteCentreImage = of.Image()
local tournetteAnneauImage = of.Image()


function Visualizer.setup()
	Logger:warn("Setup")

    flipOfflineImage:load("Images/FlipOffline.png")
    flipClosedImage:load("Images/FlipClosed.png")
    flipOpeningImage:load("Images/FlipOpening.png")
    flipOpenImage:load("Images/FlipOpen.png")
    flipRaisingImage:load("Images/FlipRaising.png")
    flipRaisedImage:load("Images/FlipRaised.png")
    tournetteCentreImage:load("Images/TournetteCentre.png")
    tournetteAnneauImage:load("Images/TournetteAnneau.png")
    
    lameVideoFace =             Environnement.getMachine("Lames Vidéo"):getAnimatable("Face");
    lameVideoLointain =           Environnement.getMachine("Lames Vidéo"):getAnimatable("Lointain");
    periacteLointainJardin =    Environnement.getMachine("Périactes Lointain"):getAnimatable("Jardin");
    periacteLointainCour =      Environnement.getMachine("Périactes Lointain"):getAnimatable("Cour");
    periacteMilieuJardin =      Environnement.getMachine("Périactes Milieu"):getAnimatable("Jardin");
    periacteMilieuCour =        Environnement.getMachine("Périactes Milieu"):getAnimatable("Cour");
    periacteFaceJardin =        Environnement.getMachine("Périactes Face"):getAnimatable("Jardin");
    periacteFaceCour =          Environnement.getMachine("Périactes Face"):getAnimatable("Cour");

    Flip_GJ2 =  Environnement.getMachine("Flip G-J2"):getAnimatable("State")
    Flip_GJ1 =  Environnement.getMachine("Flip G-J1"):getAnimatable("State")
    Flip_Gaxe = Environnement.getMachine("Flip G-Axe"):getAnimatable("State")
    Flip_GC1 =  Environnement.getMachine("Flip G-C1"):getAnimatable("State")
    Flip_GC2 =  Environnement.getMachine("Flip G-C2"):getAnimatable("State")
    Flip_EJ3 =  Environnement.getMachine("Flip E-J3"):getAnimatable("State")
    Flip_EJ2 =  Environnement.getMachine("Flip E-J2"):getAnimatable("State")
    Flip_EC2 =  Environnement.getMachine("Flip E-C2"):getAnimatable("State")
    Flip_EC3 =  Environnement.getMachine("Flip E-C3"):getAnimatable("State")
    Flip_CJ3 =  Environnement.getMachine("Flip C-J3"):getAnimatable("State")
    Flip_CJ2 =  Environnement.getMachine("Flip C-J2"):getAnimatable("State")
    Flip_CC2 =  Environnement.getMachine("Flip C-C2"):getAnimatable("State")
    Flip_CC3 =  Environnement.getMachine("Flip C-C3"):getAnimatable("State")
    Flip_AJ3 =  Environnement.getMachine("Flip A-J3"):getAnimatable("State")
    Flip_AJ2 =  Environnement.getMachine("Flip A-J2"):getAnimatable("State")
    Flip_AJ1 =  Environnement.getMachine("Flip A-J1"):getAnimatable("State")
    Flip_Aaxe = Environnement.getMachine("Flip A-Axe"):getAnimatable("State")
    Flip_AC1 =  Environnement.getMachine("Flip A-C1"):getAnimatable("State")
    Flip_AC2 =  Environnement.getMachine("Flip A-C2"):getAnimatable("State")
    Flip_AC3 =  Environnement.getMachine("Flip A-C3"):getAnimatable("State")
    flipStates = Flip_AC3:getStates()

    tournetteAnneau =   Environnement.getMachine("Tournettes"):getAnimatable("Anneau")
    tournetteCentre =   Environnement.getMachine("Tournettes"):getAnimatable("Centre")

    costiereJardin =    Environnement.getMachine("Cost Jardin"):getAnimatable("Position")
    costiereCour =      Environnement.getMachine("Cost Cour"):getAnimatable("Position")

    volJardin = Environnement.getMachine("Vol Jardin"):getAnimatable("Position")
    volCour =   Environnement.getMachine("Vol Cour"):getAnimatable("Position")

    romaine =   Environnement.getMachine("Romaine"):getAnimatable("State")
end


--local drawingMinX = -11520
--local drawingMinY = -8562.5
--local drawingMaxX = 11520
--local drawingMaxY = 8562.5
--local drawingMargin = 500.0

local drawingMinX = -14520
local drawingMinY = -10438
local drawingMaxX = 14520
local drawingMaxY = 10712
local drawingMargin = 200.0

drawingMinX = drawingMinX - drawingMargin
drawingMinY = drawingMinY - drawingMargin
drawingMaxX = drawingMaxX + drawingMargin
drawingMaxY = drawingMaxY + drawingMargin
local drawingSizeX = drawingMaxX - drawingMinX
local drawingSizeY = drawingMaxY - drawingMinY
local aspectRatio = drawingSizeX / drawingSizeY



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


    of.setColor(255)
    for i=0,(flipCount-1) do
        local imageX = minX + i * flipSizeX
        image:draw(imageX, minY, flipSizeX, sizeY)
    end

end

function drawCostiere(posX, zeroY, rangeY, animatable)
    local position_meters = animatable:getActualValue().Position
    local posY = zeroY + position_meters * 1000
    local sizeX = 3150
    local sizeY = 4150
    local lineWidth = 50
    of.setRectMode(of.RECTMODE_CORNER)
    of.setColor(255, 255, 255, 63)
    of.drawRectangle(posX - lineWidth * 0.5, zeroY, lineWidth, rangeY)

    of.setRectMode(of.RECTMODE_CENTER)
    of.setColor(255, 255, 255, 200)
    of.drawRectangle(posX, posY, sizeX, sizeY)
    of.setColor(0)
    of.drawRectangle(posX, posY, 200, 200)
    --of.drawCircle(posX, posY, 100, 100)
end

function periacteGraphic(x, y, mirror, anchorX, anchorY)
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
    of.setColor(255, 255, 255, 100)
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
    periacteGraphic(pos1, posY, orientation1, 1, 0.5)
    periacteGraphic(pos2, posY, orientation2, 0, 0.5)
end

function drawLames(posY, rangeX, animatable)
    local lineThickness = 50
    of.setColor(255, 255, 255, 63)
    of.setRectMode(of.RECTMODE_CENTER)
    of.drawRectangle(0, posY, rangeX * 2, lineThickness)

    local posX = animatable:getActualValue().Position * 1000
    local lameThickness = 100
    local rectY = posY - lameThickness * 0.5
    of.setColor(0)
    of.setRectMode(of.RECTMODE_CORNER)
    of.drawRectangle(posX, rectY, 1000, lameThickness)
    of.drawRectangle(posX + 2000, rectY, 1000, lameThickness)
    of.drawRectangle(- 1000 - posX, rectY, 1000, lameThickness)
    of.drawRectangle(- 3000 - posX, rectY, 1000, lameThickness)
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

    of.background(0)
    of.pushMatrix()
    of.scale(scaling)
    of.translate(translationX, translationY)

    drawStage()

    of.popMatrix()

end


function drawStage()

    --of.setColor(0,0,0,64)
    --of.drawRectangle(drawingMinX, drawingMinY, drawingSizeX, drawingSizeY)

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
    of.vertex(14520, -10438)
    of.vertex(-14520, -10438)
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
    local tournetteAnneauRotation = tournetteAnneau:getActualValue().Position
    local tournetteCentreRotation = tournetteCentre:getActualValue().Position
    of.setRectMode(of.RECTMODE_CENTER)
    of.pushMatrix()
    of.translate(0, 3563)
    of.rotateZDeg(tournetteCentreRotation)
    tournetteCentreImage:draw(0, 0, 7600, 7600)
    of.popMatrix()
    of.pushMatrix()
    of.translate(0, 3563)
    of.rotateZDeg(tournetteAnneauRotation)
    tournetteAnneauImage:draw(0, 0, 7600, 7600)
    of.popMatrix()


    --costieres
    drawCostiere(-6510, -7640, 12100, costiereJardin)
    drawCostiere(6510, -7640, 12100, costiereCour)

    drawPeriactes(0, 12850, periacteLointainJardin, periacteLointainCour, true, false)
    drawPeriactes(3000, 12850, periacteMilieuJardin, periacteMilieuCour, true, true)
    drawPeriactes(6002, 12850, periacteFaceJardin, periacteFaceCour, false, false)

    drawLames(4262, 11200, lameVideoLointain)
    drawLames(4462, 11200, lameVideoFace)
end


function Visualizer.exit()
    Logger:warn("Exit")
end

return Visualizer