
local Tj2
local Tj1
local Tc1
local Tc2

local halt_Tj2
local halt_Tj1
local halt_Tc1
local halt_Tc2

local Pj2
local Pj1
local Pc1
local Pc2

local halt_Pj2
local halt_Pj1
local halt_Pc1
local halt_Pc2

local overlap_j2
local overlap_j1
local overlap_c1
local overlap_c2

local finDeCourseJ2
local finDeCourseJ1
local finDeCourseC1
local finDeCourseC2

function setup()

	local tournette_J2 = Environnement.getMachine("Tournette J2");
	local tournette_J1 = Environnement.getMachine("Tournette J1");
	local tournette_C1 = Environnement.getMachine("Tournette C1");
	local tournette_C2 = Environnement.getMachine("Tournette C2");
	
	Tj2 = tournette_J2:getAnimatable("Position")
	Tj1 = tournette_J1:getAnimatable("Position")
	Tc1 = tournette_C1:getAnimatable("Position")
	Tc2 = tournette_C2:getAnimatable("Position")

	halt_Tj2 = Tj2:createHaltConstraint("Overlap")
	halt_Tj1 = Tj1:createHaltConstraint("Overlap")
	halt_Tc1 = Tc1:createHaltConstraint("Overlap")
	halt_Tc2 = Tc2:createHaltConstraint("Overlap")
	
	local pratiquable_J2 = Environnement.getMachine("Pratos J2")
	local pratiquable_J1 = Environnement.getMachine("Pratos J1")
	local pratiquable_C1 = Environnement.getMachine("Pratos C1")
	local pratiquable_C2 = Environnement.getMachine("Pratos C2")
	
	Pj2 = pratiquable_J2:getAnimatable("Velocity")
	Pj1 = pratiquable_J1:getAnimatable("Velocity")
	Pc1 = pratiquable_C1:getAnimatable("Velocity")
	Pc2 = pratiquable_C2:getAnimatable("Velocity")

	halt_Pj2 = Pj2:createHaltConstraint("Overlap")
	halt_Pj1 = Pj1:createHaltConstraint("Overlap")
	halt_Pc1 = Pc1:createHaltConstraint("Overlap")
	halt_Pc2 = Pc2:createHaltConstraint("Overlap")
	
	overlap_j2 = tournette_J2:getNodePin("Overlap")
	overlap_j1 = tournette_J1:getNodePin("Overlap")
	overlap_c1 = tournette_C1:getNodePin("Overlap")
	overlap_c2 = tournette_C2:getNodePin("Overlap")

	finDeCourseJ2 = pratiquable_J2:getNodePin("FinDeCourse")
	finDeCourseJ1 = pratiquable_J1:getNodePin("FinDeCourse")
	finDeCourseC1 = pratiquable_C1:getNodePin("FinDeCourse")
	finDeCourseC2 = pratiquable_C2:getNodePin("FinDeCourse")

end


function anticollision(tournetteAnimatable, overlapNodePin, finDeCourseNodePin, haltTournette, haltPratos)

	local f_tpos = tournetteAnimatable:getActualValue().Position
	local f_tposMax = tournetteAnimatable:getUpperPositionLimit()
	local f_tposMin = tournetteAnimatable:getLowerPositionLimit()
	local f_tolerance = 0.05

	local b_aligned
	if f_tpos < f_tolerance and f_tpos > -f_tolerance then
		b_aligned = true
	elseif f_tpos < f_tposMax + f_tolerance and f_tpos > f_tposMax - f_tolerance then
		b_aligned = true
	elseif f_tpos < f_tposMin + f_tolerance and f_tpos > f_tposMin - f_tolerance then
		b_aligned = true
	else
		b_aligned = false
	end

	local b_overlapping = overlapNodePin:getBoolValue()
	local b_finDeCourse = finDeCourseNodePin:getBoolValue()

	--Logger:warn(b_overlapping and not b_aligned)
	haltPratos:setEnabled(b_overlapping and not b_aligned)
	haltTournette:setEnabled(b_overlapping and b_aligned and not b_finDeCourse)
	 

end


--Perform Environnement Logic here :

function update()
	anticollision(Tj2, overlap_j2, finDeCourseJ2, halt_Tj2, halt_Pj2)
	anticollision(Tj1, overlap_j1, finDeCourseJ1, halt_Tj1, halt_Pj1)
	anticollision(Tc1, overlap_c1, finDeCourseC1, halt_Tc1, halt_Pc1)
	anticollision(Tc2, overlap_c2, finDeCourseC2, halt_Tc2, halt_Pc2)
end


--Clean Up stuff here :

function exit()
end




