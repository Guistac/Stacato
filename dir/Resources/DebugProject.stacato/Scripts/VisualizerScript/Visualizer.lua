
--visualizer script
local Visualizer = {}


local lameVideoFace
local lameVideoMilieu
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


function Visualizer.setup()
	Logger:warn("Setup")

    flipOfflineImage:load("Images/FlipOffline.png")
    flipClosedImage:load("Images/FlipClosed.png")
    flipOpeningImage:load("Images/FlipOpening.png")
    flipOpenImage:load("Images/FlipOpen.png")
    flipRaisingImage:load("Images/FlipRaising.png")
    flipRaisedImage:load("Images/FlipRaised.png")
    
    lameVideoFace =             Environnement.getMachine("Lames Vidéo"):getAnimatable("Face");
    lameVideoMilieu =           Environnement.getMachine("Lames Vidéo"):getAnimatable("Lointain");
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



function drawFlip(minX, minY, sizeX, sizeY, flipCount, flipState, bright)
    if bright then of.setColor(100) else of.setColor(120) end
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

    --flips row G
    drawFlip(-8480, -1438, 3960, 1000, 4, Flip_GJ2, true)
    drawFlip(-4520, -1438, 3040, 1000, 3, Flip_GJ1, false)
    drawFlip(-1480, -1438, 2960, 1000, 3, Flip_Gaxe, true)
    drawFlip(1480, -1438, 3040, 1000, 3, Flip_GC1, false)
    drawFlip(4520, -1438, 3960, 1000, 4, Flip_GC2, true)

    --flips row E
    drawFlip(-11520, 1562, 3040, 1000, 3, Flip_EJ3, true)
    drawFlip(-8480, 1562, 3960, 1000, 4, Flip_EJ2, false)
    drawFlip(4520, 1562, 3960, 1000, 4, Flip_EC2, true)
    drawFlip(8480, 1562, 3040, 1000, 3, Flip_EC3, false)

    --flips row C
    drawFlip(-11520, 4562, 3040, 1000, 3, Flip_CJ3, false)
    drawFlip(-8480, 4562, 3960, 1000, 4, Flip_CJ2, true)
    drawFlip(4520, 4562, 3960, 1000, 4, Flip_CC2, false)
    drawFlip(8480, 4562, 3040, 1000, 3, Flip_CC3, true)

    --flips row A
    drawFlip(-11520, 7562, 3040, 1000, 3, Flip_AJ3, true)
    drawFlip(-8480, 7562, 3960, 1000, 4, Flip_AJ2, false)
    drawFlip(-4520, 7562, 3040, 1000, 3, Flip_AJ1, true)
    drawFlip(-1480, 7562, 2960, 1000, 3, Flip_Aaxe, false)
    drawFlip(1480, 7562, 3040, 1000, 3, Flip_AC1, true)
    drawFlip(4520, 7562, 3960, 1000, 4, Flip_AC2, false)
    drawFlip(8480, 7562, 3040, 1000, 3, Flip_AC3, true)

    --tournettes
    of.setRectMode(of.RECTMODE_CENTER)
    of.setCircleResolution(64)
    of.setColor(200)
    of.drawCircle(0, 3563, 3800) -- center & radius
    of.setColor(255)
    of.drawCircle(0, 3563, 2800) -- center & radius

    --costieres
    of.setColor(255)
    of.drawLine(-6510, -7733, -6510, 4487)
    of.drawLine(6510, -7733, 6510, 4487)
    
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