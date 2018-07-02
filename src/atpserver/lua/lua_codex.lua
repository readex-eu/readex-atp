--[[ 
-- @file lua_codex.lua 
-- 
-- @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
-- 
-- @brief The code in this file executes a generated LUA code
--        
-- @date 01/04/2017 
--]]



--[[
-- @brief writes a piece of LUA code given as argument 
-- into a file 
--
-- @param code the code to write into a file
-- @param outfile the name of the file to write the code into
-- @return nothing 
--]]
function write_lua_code(code, outfile)

 local header = "function max(num1, num2)\n" ..
                 "  if(num1 >= num2) then\n" ..
                 "    return num1\n"..
                 "  else\n"..
                 "    return num2\n"..
                 "  end\n"..
                 "end\n\n"..
                 "function min(num1, num2)\n"..
                 "  if(num1 <= num2) then\n"..
                 "    return num1\n"..
                 "  else\n"..
                 "    return num2\n"..
                 "  end\n"..
                 "end\n"

 code = header .. "\n\n" .. code
 local f = io.open(outfile,"w")
 if f == nil then
   print("Cannot open " .. outfile .. "for writing")
   print("quitting...")
   os.exit()   
 end
 f:write(code)
 --print("--<<TRANSLATED CODE>> \n\n" .. code)
 f:close()  

end


--[[
-- @brief executes a LUA code with the dofile() API function.
-- the code executed is a loop nest which generates an array
-- of tuples
--
-- @param outfile the filename of the file containing the code
-- @return tuples an array of valid tuples of values
--         num_tuple_elems the number of tuples in the array 
--         num_tuples  the number of valus per tuple
--]]
function execute_lua_code(outFile)

 dofile(outFile)
 -- the tuple should be available here in the form
 -- tuples = { {tuple 1}, {tuple 2} ...} 
 local num_tuple_elems = 0
 local num_tuples = 0

 for k,v in pairs(tuples) do
     num_tuple_elems = #v
     local tuple_str = k .. ":" 

     for i=1,num_tuple_elems do
         tuple_str = tuple_str .. v[i] .. " "  
     end
     num_tuples = num_tuples + 1
 end  
   
 return tuples, num_tuple_elems, num_tuples
end




