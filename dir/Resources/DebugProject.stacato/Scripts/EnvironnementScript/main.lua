
local Tj2
local Tj1
local Tc1
local Tc2

local Pj2
local Pj1
local Pc1
local Pc2

local overlap_Pj2
local overlap_Pj1
local overlap_Pc1
local overlap_Pc2

function setup()

	local tournette_J2 = Environnement.getMachine("Tournette J2");
	local tournette_J1 = Environnement.getMachine("Tournette J1");
	local tournette_C1 = Environnement.getMachine("Tournette C1");
	local tournette_C2 = Environnement.getMachine("Tournette C2");
	
	Tj2 = tournette_J2:getAnimatable("Position")
	Tj1 = tournette_J1:getAnimatable("Position")
	Tc1 = tournette_C1:getAnimatable("Position")
	Tc2 = tournette_C2:getAnimatable("Position")
	
	local pratiquable_J2 = Environnement.getMachine("Pratiquable J2")
	local pratiquable_J1 = Environnement.getMachine("Pratiquable J1")
	local pratiquable_C1 = Environnement.getMachine("Pratiquable C1")
	local pratiquable_C2 = Environnement.getMachine("Pratiquable C2")
	
	Pj2 = pratiquable_J2:getAnimatable("Position")
	Pj1 = pratiquable_J1:getAnimatable("Position")
	Pc1 = pratiquable_C1:getAnimatable("Position")
	Pc2 = pratiquable_C2:getAnimatable("Position")
	
	overlap_Pj2 = pratiquable_J2:getNodePin("Overlap")
	overlap_Pj1 = pratiquable_J1:getNodePin("Overlap")
	overlap_Pc1 = pratiquable_C1:getNodePin("Overlap")
	overlap_Pc2 = pratiquable_C2:getNodePin("Overlap")

end


--Perform Environnement Logic here :

function update()

	local o1 = overlap_Pj2:getBoolValue()
	local o2 = overlap_Pj1:getBoolValue()
	local o3 = overlap_Pc1:getBoolValue()
	local o4 = overlap_Pc2:getBoolValue()

	--Logger:warn("J2-" .. tostring(o1) .. " J1-" .. tostring(o2) .. " C1-" .. tostring(o3) .. " C2-" .. tostring(o4))

end


--Clean Up stuff here :

function exit()
end






