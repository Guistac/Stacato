
--Machines
local flipGC2
local flipEC2
local flipCC2
local flipGJ2
local flipEJ2
local flipCJ2
local flipStates;
local costiereCour
local costiereJardin
local periacteFaceCour
local periacteFaceJardin
local periacteMilieuCour
local periacteMilieuJardin
local periacteLointainCour
local periacteLointainJardin
local lamesFace
local lamesLointain

--Constraints
local flipGC2_Halt
local flipEC2_Halt
local flipCC2_Halt
local flipGJ2_Halt
local flipEJ2_Halt
local flipCJ2_Halt
local costiereCour_keepoutGC2
local costiereCour_keepoutEC2
local costiereCour_keepoutCC2
local costiereJardin_keepoutGJ2
local costiereJardin_keepoutEJ2
local costiereJardin_keepoutCJ2



local costiereCour_keepoutPLointain
local costiereCour_keepoutPMilieu
local costiereCour_keepoutPMilieu
local costiereCour_keepoutLames

local costiereJardin_keepoutPLointain
local costiereJardin_keepoutPMilieu
local costiereJardin_keepoutPFace
local costiereJardin_keepoutLames



local lointainCour_keepoutCostiereCour
local lointainCour_keepoutCostiereJardin
local lointainJardin_keepoutCostiereCour
local lointainJardin_keepoutCostiereJardin

local pMilieuCour_keepoutCostiereCour
local pMilieuCour_keepoutCostiereJardin
local pMilieuJardin_keepoutCostiereCour
local pMilieuJardin_keepoutCostiereJardin

local pFaceCour_keepoutCostiereCour
local pFaceCour_keepoutCostiereJardin
local pFaceJardin_keepoutCostiereCour
local pFaceJardin_keepoutCostiereJardin

local lamesLointain_keepoutCostiereCour
local lamesLointain_keepoutCostiereJardin
local lamesFace_keepoutCostiereCour
local lamesFace_keepoutCostiereJardin


--periactes cour keepout costiere cour		(3.25	8.7)
--periactes jardin keepout costiere cour	(4.5	10.0)
--lames keepout costiere cour				(1.60	8.7)

--periactes jardin keepout costiere jardin	(-8.7 	-3.2)
--periactes cour keepout cosieter cour		(-10.0	-4.5)
--lames keepout costiere jardin				(1.4	8.6)


--periactes lointain keepout	(4.8	10.5)
--periactes milieu keepout		(7.65	999)
--periactes face keepout		(10.75	999)
--lames lointain keepout		(9.35	999)
--lames face keepout			(9.5	999)

function setup()

	--Get Machines
	flipGC2 = Environnement.getMachine("Flip G-C2"):getAnimatable("State")
	flipEC2 = Environnement.getMachine("Flip E-C2"):getAnimatable("State")
	flipCC2 = Environnement.getMachine("Flip C-C2"):getAnimatable("State")
	flipGJ2 = Environnement.getMachine("Flip G-J2"):getAnimatable("State")
	flipEJ2 = Environnement.getMachine("Flip E-J2"):getAnimatable("State")
	flipCJ2 = Environnement.getMachine("Flip C-J2"):getAnimatable("State")
	flipStates = flipGC2:getStates()
	periacteFaceCour = Environnement.getMachine("Périactes Face"):getAnimatable("Cour")
	periacteFaceJardin = Environnement.getMachine("Périactes Face"):getAnimatable("Jardin")
	periacteMilieuCour = Environnement.getMachine("Périactes Milieu"):getAnimatable("Cour")
	periacteMilieuJardin = Environnement.getMachine("Périactes Milieu"):getAnimatable("Jardin")
	periacteLointainCour = Environnement.getMachine("Périactes Lointain"):getAnimatable("Cour")
	periacteLointainJardin = Environnement.getMachine("Périactes Lointain"):getAnimatable("Jardin")
	lamesFace = Environnement.getMachine("Lames Vidéo"):getAnimatable("Face")
	lamesLointain = Environnement.getMachine("Lames Vidéo"):getAnimatable("Lointain")
	costiereCour = Environnement.getMachine("Cost Cour"):getAnimatable("Position")
	costiereJardin = Environnement.getMachine("Cost Jardin"):getAnimatable("Position")

	--Flip Constraints
	flipGC2_Halt = flipGC2:createHaltConstraint("Anti-Collision Costière Cour")
	flipEC2_Halt = flipEC2:createHaltConstraint("Anti-Collision Costière Cour")
	flipCC2_Halt = flipCC2:createHaltConstraint("Anti-Collision Costière Cour")
	flipGJ2_Halt = flipGJ2:createHaltConstraint("Anti-Collision Costière Jardin")
	flipEJ2_Halt = flipEJ2:createHaltConstraint("Anti-Collision Costière Jardin")
	flipCJ2_Halt = flipCJ2:createHaltConstraint("Anti-Collision Costière Jardin")

	--Costières Constraints
	costiereCour_keepoutGC2 = costiereCour:createKeepoutConstraint("Keepout G-C2", 4.12, 9.27)
	costiereCour_keepoutEC2 = costiereCour:createKeepoutConstraint("Keepout E-C2", 7.12, 99.9)
	costiereCour_keepoutCC2 = costiereCour:createKeepoutConstraint("Keepout C-C2", 10.10, 99.9)
	costiereJardin_keepoutGJ2 = costiereJardin:createKeepoutConstraint("Keepout G-J2", 4.10, 9.28)
	costiereJardin_keepoutEJ2 = costiereJardin:createKeepoutConstraint("Keepout E-J2", 7.13, 99.9)
	costiereJardin_keepoutCJ2 = costiereJardin:createKeepoutConstraint("Keepout C-J2", 10.12, 99.9)
	costiereCour_keepoutPLointain = 	costiereCour:createKeepoutConstraint("Keepout Périactes Lointain",	0.0,	0.0)
	costiereCour_keepoutPMilieu = 		costiereCour:createKeepoutConstraint("Keepout Périactes Milieu",	0.0,	0.0)
	costiereCour_keepoutPFace = 		costiereCour:createKeepoutConstraint("Keepout Périactes Face",		0.0,	0.0)
	costiereCour_keepoutLames = 		costiereCour:createKeepoutConstraint("Keepout Lames Vidéo",			0.0,	0.0)
	costiereJardin_keepoutPLointain =	costiereJardin:createKeepoutConstraint("Keepout Périactes Lointain",0.0,	0.0)
	costiereJardin_keepoutPMilieu =		costiereJardin:createKeepoutConstraint("Keepout Périactes Milieu",	0.0,	0.0)
	costiereJardin_keepoutPFace =		costiereJardin:createKeepoutConstraint("Keepout Périactes Face",	0.0,	0.0)
	costiereJardin_keepoutLames =		costiereJardin:createKeepoutConstraint("Keepout Lames Vidéo",		0.0,	0.0)
	
	--Periactes Lointain Constraints
	lointainCour_keepoutCostiereCour =		periacteLointainCour:createKeepoutConstraint("Keepout Costière Cour",		0.0,	0.0)
	lointainCour_keepoutCostiereJardin =	periacteLointainCour:createKeepoutConstraint("Keepout Costière Jardin",		0.0,	0.0)
	lointainJardin_keepoutCostiereCour =	periacteLointainJardin:createKeepoutConstraint("Keepout Costière Cour",		0.0,	0.0)
	lointainJardin_keepoutCostiereJardin =	periacteLointainJardin:createKeepoutConstraint("Keepout Costière Jardin",	0.0,	0.0)
	
	--Periactes Milieu Constraints
	pMilieuCour_keepoutCostiereCour =		periacteMilieuCour:createKeepoutConstraint("Keepout Costière Cour",			0.0,	0.0)
	pMilieuCour_keepoutCostiereJardin =		periacteMilieuCour:createKeepoutConstraint("Keepout Costière Jardin",		0.0,	0.0)
	pMilieuJardin_keepoutCostiereCour =		periacteMilieuJardin:createKeepoutConstraint("Keepout Costière Cour",		0.0,	0.0)
	pMilieuJardin_keepoutCostiereJardin =	periacteMilieuJardin:createKeepoutConstraint("Keepout Costière Jardin",		0.0,	0.0)
	
	--Periactes Face Constraints
	pFaceCour_keepoutCostiereCour =			periacteFaceCour:createKeepoutConstraint("Keepout Costière Cour",			0.0,	0.0)
	pFaceCour_keepoutCostiereJardin =		periacteFaceCour:createKeepoutConstraint("Keepout Costière Jardin",			0.0,	0.0)
	pFaceJardin_keepoutCostiereCour =		periacteFaceJardin:createKeepoutConstraint("Keepout Costière Cour",			0.0,	0.0)
	pFaceJardin_keepoutCostiereJardin =		periacteFaceJardin:createKeepoutConstraint("Keepout Costière Jardin",		0.0,	0.0)
	
	--Lames Video Constraints
	lamesLointain_keepoutCostiereCour =		lamesLointain:createKeepoutConstraint("Keepout Costière Cour", 				0.0,	0.0)
	lamesLointain_keepoutCostiereJardin =	lamesLointain:createKeepoutConstraint("Keepout Costière Jardin", 			0.0,	0.0)
	lamesFace_keepoutCostiereCour =			lamesFace:createKeepoutConstraint("Keepout Costière Cour", 					0.0,	0.0)
	lamesFace_keepoutCostiereJardin =		lamesFace:createKeepoutConstraint("Keepout Costière Jardin", 				0.0,	0.0)


end


function update()

	------- Costiere Cour // Flipts GC2 EC2 CC2
	costiereCour_keepoutGC2:setEnabled(flipGC2:getActualValue() ~= flipStates.Closed or not flipGC2:isOnline())
	costiereCour_keepoutEC2:setEnabled(flipEC2:getActualValue() ~= flipStates.Closed or not flipEC2:isOnline())
	costiereCour_keepoutCC2:setEnabled(flipCC2:getActualValue() ~= flipStates.Closed or not flipCC2:isOnline())
	costiereJardin_keepoutGJ2:setEnabled(flipGJ2:getActualValue() ~= flipStates.Closed or not flipGJ2:isOnline())
	costiereJardin_keepoutEJ2:setEnabled(flipEJ2:getActualValue() ~= flipStates.Closed or not flipEJ2:isOnline())
	costiereJardin_keepoutCJ2:setEnabled(flipCJ2:getActualValue() ~= flipStates.Closed or not flipCJ2:isOnline())

	local costCour_pos = costiereCour:getActualValue().Position
	local costCour_brk = costiereCour:getBrakingPosition()
	local costJardin_pos = costiereJardin:getActualValue().Position
	local costJardin_brk = costiereJardin:getBrakingPosition()

	flipGC2_Halt:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk, 4.17, 9.22) or not costiereCour:isOnline())
	flipEC2_Halt:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk, 7.17, 99.9) or not costiereCour:isOnline())
	flipCC2_Halt:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk, 10.15, 99.9) or not costiereCour:isOnline())
	flipGJ2_Halt:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk, 4.15, 9.23) or not costiereJardin:isOnline())
	flipEJ2_Halt:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk, 7.18, 99.9) or not costiereJardin:isOnline())
	flipCJ2_Halt:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk, 10.17, 99.9) or not costiereJardin:isOnline())

end


--Clean Up stuff here :

function exit()
end

















