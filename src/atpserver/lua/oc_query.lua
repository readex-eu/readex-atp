--[[ 
-- @file oc_query.lua
-- 
-- @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
-- 
-- @brief The code in this file enables to query the OC with two requests: 
--        code generation and point validation
-- 
-- Provides functions to query the OC and fetch its response. Two types 
-- of requests are taken into account:
-- 1- code generation (codegen function): OC is requested to generate a code
-- which if executed produces tuples of valid points for a given number of
-- entry parameters whith their ranges and constraiants.
-- 2- point validation (pointvad): given a tuple of values and constraints
-- corresponding to a set of parameters, the OC is requested to tell if the
-- tuple of values is valid or not        
-- 
-- Code generation constitutes the prefered query function as it produces 
-- directly the list of valud tuple of points, but in case of failure of
-- OC to generate the code, the point validation would be the substitute. 
--
-- It should be noted that the requests are built in C code and passsed to 
-- LUA through the LUA-C interface.
-- @date 01/04/2017
--]]

local main_path = os.getenv("ATP_PATH")
local oc_path = os.getenv("ATP_OC_PATH")
package.path = main_path .. "/?.lua;" .. package.path 

require  "lua.parse_oc_code"
require  "lua.lua_codex"

--[[
-- @brief launches the Omega Calculator (OC) with a given request. It
-- if the request succeeeds the function returns the OC response 
--
-- @param request the request to submit to OC
-- @return the output text of OC
--]]
function launch_oc(request)
    
    local ocOutputFilePath = "oc_query.out"
    local ocInputFilePath = "oc_query.in"
    if (oc_path ~= nil) then
       ocOutputFilePath = oc_path .. "/oc_query.out"
       ocInputFilePath = oc_path .. "/oc_query.in"    
    end    
    
    local inputFile, errorMsg = io.open(ocInputFilePath,"w")
    if(inputFile) then
        inputFile:write(request)
        inputFile:close() 
    else 
        print("ATP lua ERROR: Could not write to file (" .. errorMsg .. ")")
        print("quitting ...") 
        os.exit()
    end
      
    os.execute(main_path .. "/lua/oc " .. ocInputFilePath .. " > " .. ocOutputFilePath)
    local file, errorMsg = io.open(ocOutputFilePath, "r")
    if( file == nil) then 
        print("ATP lua ERROR: No output from OC (" .. errorMsg .. ")")
        print("quitting ...") 
        os.exit()
    end  
    local text = ""
    while true do
        local line = file:read()
        if line == nil then break end
        if string.sub(line,1,1) ~= "#" then
            text = text .. line .. "\n"
        end
    end
    --print(text)
    file:close()
    return text 
end  

--[[
-- @brief OC is requested to generate a code which if 
-- executed produces tuples of valid points for a given 
-- number of entry parameters whith their ranges and 
-- constraiants. The received OC generated code
-- is parsed and converted into LUA code through the function
-- parse_code, and then written to a LUA file with the function
-- write_lua_code to be loaded and executed by the function
-- execute_lua_code   
--
-- @param request The request to submit to OC
-- @return tuples an array of valid tuples of values
--         num_tuple_elems the number of tuples in the array 
--         num_tuples  the number of valus per tuple
--]]

function codegen(request)

  
    local outf = "gen_file.lua" 
    if (oc_path ~= nil) then
       outf = oc_path .. "/gen_file.lua"
    end
    code = launch_oc(request)
    --------------------------------------------
    -- OC code generation ----------------------
    new_code = parse_code(code)
    --print(new_code)
    write_lua_code(new_code,outf)
    tuples, num_tuple_elems, num_tuples = 
                            execute_lua_code(outf)
    return tuples , num_tuple_elems, num_tuples 
end


--[[
-- @brief point validation (pointvad): given a tuple of values and constraints
-- corresponding to a set of parameters, the OC is requested to tell if the
-- tuple of values is valid or not        
--
-- @param request the request to submit ot OC
-- return 1 if the tuple is valid, 0 otherwise
--]]
function pointvad(request)

    code = launch_oc(request)    
    --------------------------------------------
    -- OC point validation ---------------------
    local valid = 0  
    for line in code:gmatch("([^\r\n]*)[\r\n]?") do    
    	local s,e = string.find(line,"TRUE")

    	if s  ~= nil and e ~= nil then
       	   valid = 1    
	   break
	end
    end
    return valid
end
