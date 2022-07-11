
---------------------------------------
----Default Environnement Script----
---------------------------------------

--Initialize stuff here :

local costiereA_Position
local costiereA_constraintA

function setup()

	local costiereA = Environnement.getMachine("Costière A")
	costiereA_Position = costiereA:getAnimatable("Position")
	costiereA_constraintA = costiereA_Position:createKeepoutConstraint("KeepoutA", 2.3, 9.3)

end


--Perform Environnement Logic here :

local test = 0
function update()
test = test + 1
--Logger:info(test)
end


--Clean Up stuff here :

function exit()
end

























