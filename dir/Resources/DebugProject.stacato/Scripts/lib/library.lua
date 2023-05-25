local M = {}

local testVariable = 0

function M.increment()
	testVariable = testVariable + 1
	Logger:warn("Testing", testVariable)
end

function M.get()
	return testVariable
end

return M