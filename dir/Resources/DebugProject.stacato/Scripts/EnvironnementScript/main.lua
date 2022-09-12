
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
local costiereCour_keepoutPFace
local costiereCour_keepoutLamesLointain
local costiereCour_keepoutLamesFace

local costiereJardin_keepoutPLointain
local costiereJardin_keepoutPMilieu
local costiereJardin_keepoutPFace
local costiereJardin_keepoutLamesLointain
local costiereJardin_keepoutLamesFace


local pLointainCour_keepoutCostiereCour
local pLointainCour_keepoutCostiereJardin
local pLointainJardin_keepoutCostiereCour
local pLointainJardin_keepoutCostiereJardin

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

local counter = 0

--periactes cour keepout costiere cour		(3.25	8.7)
--periactes jardin keepout costiere cour	(4.5	10.0)
--lames keepout costiere cour				(1.60	8.7)

--periactes jardin keepout costiere jardin	(-8.7 	-3.2)
--periactes cour keepout cosieter cour		(-10.0	-4.5)
--lames keepout costiere jardin				(1.4	8.6)


--costieres keepout periactes lointain	(4.8	10.5)
--costieres keepoutperiactes milieu		(7.65	999)
--costieres keepoutperiactes face		(10.75	999)
--costieres keepoutlames lointain		(9.35	999)
--costieres keepoutlames face			(9.5	999)

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
	costiereCour_keepoutGC2 =			costiereCour:createKeepoutConstraint("Keepout G-C2", 4.1, 9.3)
	costiereCour_keepoutEC2 =			costiereCour:createKeepoutConstraint("Keepout E-C2", 7.1, 99.9)
	costiereCour_keepoutCC2 =			costiereCour:createKeepoutConstraint("Keepout C-C2", 10.10, 99.9)

	costiereJardin_keepoutGJ2 =			costiereJardin:createKeepoutConstraint("Keepout G-J2", 4.1, 9.3)
	costiereJardin_keepoutEJ2 =			costiereJardin:createKeepoutConstraint("Keepout E-J2", 7.1, 99.9)
	costiereJardin_keepoutCJ2 =			costiereJardin:createKeepoutConstraint("Keepout C-J2", 10.1, 99.9)

--costieres keepout Gx2		(4.1	9.3)
--costieres keepout Ex2		(7.1	99.9)
--costieres keepout Cx2		(10.1	99.9)

	costiereCour_keepoutPLointain = 	costiereCour:createKeepoutConstraint("Keepout Périactes Lointain",	4.8,	10.5)
	costiereCour_keepoutPMilieu = 		costiereCour:createKeepoutConstraint("Keepout Périactes Milieu",	7.65,	99.9)
	costiereCour_keepoutLamesLointain =	costiereCour:createKeepoutConstraint("Keepout Lames Lointain",		9.35,	99.9)
	costiereCour_keepoutLamesFace =		costiereCour:createKeepoutConstraint("Keepout Lames Face",			9.5,	99.9)
	costiereCour_keepoutPFace = 		costiereCour:createKeepoutConstraint("Keepout Périactes Face",		10.75,	99.9)

	costiereJardin_keepoutPLointain =		costiereJardin:createKeepoutConstraint("Keepout Périactes Lointain",	4.8,	10.5)
	costiereJardin_keepoutPMilieu =			costiereJardin:createKeepoutConstraint("Keepout Périactes Milieu",		7.65,	99.9)
	costiereJardin_keepoutLamesLointain =	costiereJardin:createKeepoutConstraint("Keepout Lames Lointain",		9.35,	99.9)
	costiereJardin_keepoutLamesFace =		costiereJardin:createKeepoutConstraint("Keepout Lames Face",			9.5,	99.9)
	costiereJardin_keepoutPFace =			costiereJardin:createKeepoutConstraint("Keepout Périactes Face",		10.75,	99.9)

--costieres keepout periactes lointain	(4.8	10.5)
--costieres keepoutperiactes milieu		(7.65	999)
--costieres keepoutperiactes face		(10.75	999)
--costieres keepoutlames lointain		(9.35	999)
--costieres keepoutlames face			(9.5	999)


	pLointainCour_keepoutCostiereCour =		periacteLointainCour:createKeepoutConstraint("Keepout Costière Cour",		3.25,	8.7)
	pMilieuCour_keepoutCostiereCour =		periacteMilieuCour:createKeepoutConstraint("Keepout Costière Cour",			3.25,	8.7)
	pFaceCour_keepoutCostiereCour =			periacteFaceCour:createKeepoutConstraint("Keepout Costière Cour",			3.25,	8.7)

	pLointainJardin_keepoutCostiereCour =	periacteLointainJardin:createKeepoutConstraint("Keepout Costière Cour",		4.5,	10.0)
	pMilieuJardin_keepoutCostiereCour =		periacteMilieuJardin:createKeepoutConstraint("Keepout Costière Cour",		4.5,	10.0)
	pFaceJardin_keepoutCostiereCour =		periacteFaceJardin:createKeepoutConstraint("Keepout Costière Cour",			4.5,	10.0)

	lamesLointain_keepoutCostiereCour =		lamesLointain:createKeepoutConstraint("Keepout Costière Cour", 				1.6,	8.7)
	lamesFace_keepoutCostiereCour =			lamesFace:createKeepoutConstraint("Keepout Costière Cour", 					1.6,	8.7)

--periactes cour keepout costiere cour		(3.25	8.7)
--periactes jardin keepout costiere cour	(4.5	10.0)
--lames keepout costiere cour				(1.60	8.7)


	pLointainCour_keepoutCostiereJardin =	periacteLointainCour:createKeepoutConstraint("Keepout Costière Jardin",		-8.7,	-3.2)
	pMilieuCour_keepoutCostiereJardin =		periacteMilieuCour:createKeepoutConstraint("Keepout Costière Jardin",		-8.7,	-3.2)
	pFaceCour_keepoutCostiereJardin =		periacteFaceCour:createKeepoutConstraint("Keepout Costière Jardin",			-8.7,	-3.2)

	pLointainJardin_keepoutCostiereJardin =	periacteLointainJardin:createKeepoutConstraint("Keepout Costière Jardin",	-10.0,	-4.5)
	pMilieuJardin_keepoutCostiereJardin =	periacteMilieuJardin:createKeepoutConstraint("Keepout Costière Jardin",		-10.0,	-4.5)
	pFaceJardin_keepoutCostiereJardin =		periacteFaceJardin:createKeepoutConstraint("Keepout Costière Jardin",		-10.0,	-4.5)

	lamesLointain_keepoutCostiereJardin =	lamesLointain:createKeepoutConstraint("Keepout Costière Jardin", 			1.4,	8.6)
	lamesFace_keepoutCostiereJardin =		lamesFace:createKeepoutConstraint("Keepout Costière Jardin", 				1.4,	8.6)

--periactes jardin keepout costiere jardin	(-8.7 	-3.2)
--periactes cour keepout cosieter cour		(-10.0	-4.5)
--lames keepout costiere jardin				(1.4	8.6)

end








function update()

	local costCour_pos =		costiereCour:getActualValue().Position
	local costCour_brk =		costiereCour:getBrakingPosition()
	local costJardin_pos =		costiereJardin:getActualValue().Position
	local costJardin_brk =		costiereJardin:getBrakingPosition()
	local pLointainCour_pos =	periacteLointainCour:getActualValue().Position
	local pLointainCour_brk =	periacteLointainCour:getBrakingPosition()
	local pLointainJardin_pos =	periacteLointainJardin:getActualValue().Position
	local pLointainJardin_brk =	periacteLointainJardin:getBrakingPosition()
	local pMilieuCour_pos =		periacteMilieuCour:getActualValue().Position
	local pMilieuCour_brk =		periacteMilieuCour:getBrakingPosition()
	local pMilieuJardin_pos =	periacteMilieuJardin:getActualValue().Position
	local pMilieuJardin_brk =	periacteMilieuJardin:getBrakingPosition()
	local pFaceCour_pos =		periacteFaceCour:getActualValue().Position
	local pFaceCour_brk =		periacteFaceCour:getBrakingPosition()
	local pFaceJardin_pos =		periacteFaceJardin:getActualValue().Position
	local pFaceJardin_brk =		periacteFaceJardin:getBrakingPosition()
	local lamesLointain_pos =	lamesLointain:getActualValue().Position
	local lamesLointain_brk =	lamesLointain:getBrakingPosition()
	local lamesFace_pos =		lamesFace:getActualValue().Position
	local lamesFace_brk =		lamesFace:getBrakingPosition()

	------- Costiere Cour // Flipts GC2 EC2 CC2
	costiereCour_keepoutGC2:setEnabled(flipGC2:getActualValue() ~= flipStates.Closed or not flipGC2:isOnline())
	costiereCour_keepoutEC2:setEnabled(flipEC2:getActualValue() ~= flipStates.Closed or not flipEC2:isOnline())
	costiereCour_keepoutCC2:setEnabled(flipCC2:getActualValue() ~= flipStates.Closed or not flipCC2:isOnline())
	costiereJardin_keepoutGJ2:setEnabled(flipGJ2:getActualValue() ~= flipStates.Closed or not flipGJ2:isOnline())
	costiereJardin_keepoutEJ2:setEnabled(flipEJ2:getActualValue() ~= flipStates.Closed or not flipEJ2:isOnline())
	costiereJardin_keepoutCJ2:setEnabled(flipCJ2:getActualValue() ~= flipStates.Closed or not flipCJ2:isOnline())

	local clearance = 0.05

	flipGC2_Halt:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk, 4.1 + clearance, 			9.3 - clearance) or not costiereCour:isOnline())
	flipEC2_Halt:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk, 7.1 + clearance, 			99.9) or not costiereCour:isOnline())
	flipCC2_Halt:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk, 10.1 + clearance, 		99.9) or not costiereCour:isOnline())
	flipGJ2_Halt:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk, 4.1 + clearance, 		9.3 - clearance) or not costiereJardin:isOnline())
	flipEJ2_Halt:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk, 7.1 + clearance, 		99.9) or not costiereJardin:isOnline())
	flipCJ2_Halt:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk, 10.1 + clearance, 	99.9) or not costiereJardin:isOnline())

--costieres keepout Gx2		(4.1	9.3)
--costieres keepout Ex2		(7.1	99.9)
--costieres keepout Cx2		(10.1	99.9)99.9)


	costiereCour_keepoutPLointain:setEnabled(
		checkRangeOverlap(pLointainCour_pos, pLointainCour_brk,		3.25 + clearance,	8.7 - clearance) or
		checkRangeOverlap(pLointainJardin_pos, pLointainJardin_brk,	4.5 + clearance,	10.0 - clearance))
	costiereCour_keepoutPMilieu:setEnabled(
		checkRangeOverlap(pMilieuCour_pos, pMilieuCour_brk,			3.25 + clearance,	8.7 - clearance) or
		checkRangeOverlap(pMilieuJardin_pos, pMilieuJardin_brk,		4.5 + clearance,	10.0 - clearance))
	costiereCour_keepoutPFace:setEnabled(
		checkRangeOverlap(pFaceCour_pos, pFaceCour_brk,				3.25 + clearance,	8.7 - clearance) or
		checkRangeOverlap(pFaceJardin_pos, pFaceJardin_brk, 		4.5 + clearance,	10.0 - clearance))
	costiereCour_keepoutLamesLointain:setEnabled(
		checkRangeOverlap(lamesLointain_pos, lamesLointain_brk,		1.6 + clearance,	8.7 - clearance))
	costiereCour_keepoutLamesFace:setEnabled(
		checkRangeOverlap(lamesFace_pos, lamesFace_brk,				1.6 + clearance, 	8.7 - clearance))

--periactes cour keepout costiere cour		(3.25	8.7)
--periactes jardin keepout costiere cour	(4.5	10.0)
--lames keepout costiere cour				(1.60	8.7)




	costiereJardin_keepoutPLointain:setEnabled(
		checkRangeOverlap(pLointainCour_pos, pLointainCour_brk,		-10.0 + clearance,	-4.5 - clearance) or
		checkRangeOverlap(pLointainJardin_pos, pLointainJardin_brk,	-8.7 + clearance,	-3.2 - clearance))
	costiereJardin_keepoutPMilieu:setEnabled(
		checkRangeOverlap(pMilieuCour_pos, pMilieuCour_brk,			-10.0 + clearance,	-4.5 - clearance) or
		checkRangeOverlap(pMilieuJardin_pos, pMilieuJardin_brk,		-8.7 + clearance,	-3.2 - clearance))
	costiereJardin_keepoutPFace:setEnabled(
		checkRangeOverlap(pFaceCour_pos, pFaceCour_brk,				-10.0 + clearance,	-4.5 - clearance) or
		checkRangeOverlap(pFaceJardin_pos, pFaceJardin_brk,			-8.7 + clearance,	-3.2 - clearance))
	costiereJardin_keepoutLamesLointain:setEnabled(
		checkRangeOverlap(lamesLointain_pos, lamesLointain_brk,		1.4 + clearance,	8.6 - clearance))
	costiereJardin_keepoutLamesFace:setEnabled(
		checkRangeOverlap(lamesFace_pos, lamesFace_brk,				1.4 + clearance,	8.6 - clearance))

--periactes cour keepout cosieter jardin		(-10.0	-4.5)
--periactes jardin keepout costiere jardin	(-8.7 	-3.2)
--lames keepout costiere jardin				(1.4	8.6)


	pLointainCour_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,		4.8 + clearance,	10.5 - clearance))
	pMilieuCour_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,		7.65 + clearance,	99.9))
	pFaceCour_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,			10.75 + clearance,	99.9))

	pLointainJardin_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,	4.8 + clearance,	10.5 - clearance))
	pMilieuJardin_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,		7.65 + clearance,	99.9))
	pFaceJardin_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,		10.75 + clearance,	99.9))

	lamesLointain_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,		9.35 + clearance,	99.9))
	lamesFace_keepoutCostiereCour:setEnabled(checkRangeOverlap(costCour_pos, costCour_brk,			9.5 + clearance,	99.9))



	pLointainCour_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,	4.8 + clearance,	10.5 - clearance))
	pMilieuCour_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,		7.65 + clearance,	99.9))
	pFaceCour_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,		10.75 + clearance,	99.9))

	pLointainJardin_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,	4.8 + clearance,	10.5 - clearance))
	pMilieuJardin_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,	7.65 + clearance,	99.9))
	pFaceJardin_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,		10.75 + clearance,	99.9))

	lamesLointain_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,	9.35 + clearance,	99.9))
	lamesFace_keepoutCostiereJardin:setEnabled(checkRangeOverlap(costJardin_pos, costJardin_brk,		9.5 + clearance,	99.9))

--costieres keepout periactes lointain	(4.8	10.5)
--costieres keepout periactes milieu	(7.65	999)
--costieres keepout periactes face		(10.75	999)
--costieres keepout lames lointain		(9.35	999)
--costieres keepout lames face			(9.5	999)


end











--Clean Up stuff here :

function exit()
end
















































