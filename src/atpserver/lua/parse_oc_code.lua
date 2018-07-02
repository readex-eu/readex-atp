--[[
-- @file parse_oc_code.lua
--
-- @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
-- 
-- @brief The code in this file parses an OC generated loop nest
-- and converts it into LUA code for execution 
--
-- @data 01/04/2017
--]]

_tab = {}



--[[
-- @brief parses the C like code and converts it into LUA code 
-- 
-- @param code the code to parse and convert
-- @return new_code converted LUA code
--
--]]
function parse_code(code)   

  local new_code = "tuples = {}\n"
  local ident = ""

  for line in code:gmatch("([^\r\n]*)[\r\n]?") do

    local matched = false
    --parse for statement
    local s,e = string.find(line,"for%([^;]*; [^;]*;[^}]*%)") 
    if(s ~= nil and e ~= nil) then
      new_code = new_code .. ident .. parse_for(line)
      ident = ident .. "  " 
      matched = true
    end

    --parse if statement
    s,e = string.find(line, "if%s*%(.*%)")
    if (s ~= nul and e ~= nil) then
       new_code = new_code .. ident .. parse_if(string.sub(line,s,e))
       ident = ident .. "  "
       matched = true
    end    


    --parse end of statement (either for or if)
    s,e = string.find(line,"[^}]*}")
    if(s ~= nil and e ~= nil and not matched) then 
      ident = string.sub(ident,1,-3)
      new_code = new_code .. ident .. parse_end_stmt(line)
      matched = true
    end
   

    -- parse the output statement (Sx(tx,ty,..))
    s,e = string.find(line,"[%w%a]+%([^%)]+%);")
    if(s ~= nil and e ~= nil and not matched) then 
      new_code = new_code .. ident .. "tuples[#tuples+1] = "
                 .. parse_gen_stmt(line)
      matched = true
    end
   

    if not matched then
      new_code = new_code .. ident .. line .. 
                 "--<LINE NOT MATCHED>\n" 
    end        
 
  end

  return new_code
end



-- @brief parses a C-like for statement and convert it into a LUA for statement
function parse_for(code)

    local lua_for = "" 

    s,e = string.find(code,"for%([^;]*; [^;]*;[^}]*%)") 
    --print(string.sub(code,s,e))

    for_code = string.sub(code,s,e)
    for_code = string.sub(for_code,5,-2)
    fe,fs = string.find(for_code, "[^;]*")        
    fst_term = string.sub(for_code,fe,fs)
    for_code = string.sub(for_code,fs+2,-1)
    se,ss = string.find(for_code,"[^;]*")
    snd_term = string.sub(for_code,se,ss)
    for_code = string.sub(for_code,ss+2,-1)
    trd_term = for_code

    fst_term_lua = parse_term(fst_term,"FOR_TERM_1")
    snd_term_lua = parse_term(snd_term, "FOR_TERM_2")
    trd_term_lua = parse_term(trd_term, "FOR_TERM_3")
   
    if (trd_term_lua ~= nil) then
      lua_for = "for " .. fst_term_lua ..
                "," .. snd_term_lua ..
                "," .. trd_term_lua ..
                " do\n"
    else
      lua_for = "for " .. fst_term_lua ..
                "," .. snd_term_lua ..
                " do\n"
    end

    return lua_for
end

-- @brief parses a C-like if statement and conver it into a LUA if statement
function parse_if(code)
    local lua_if = code
    --find ||, && and != and replace with or, and and ~=
    lua_if = string.gsub(lua_if, "||", "or")
    lua_if = string.gsub(lua_if, "&&", "and")
    lua_if = string.gsub(lua_if, "!=", "~=")
    return lua_if .. " then\n"
end

-- @brief parses a C-like closing bracket and convert it into a LUA end statement
function parse_end_stmt(code)

  return "end\n"  
end


-- @brief parses a tuple reference statement and convert it to a LUA array statement
function parse_gen_stmt(code)
  
  local s,e = string.find(code,"[%w%a]+")
  local strip_code = string.sub(code,e+1,-1)
  --print(strip_code)  
  strip_code = string.gsub(strip_code,"%(","{")
  --print(strip_code)  
  strip_code = string.gsub(strip_code,"%)","}")  
  return strip_code .. "\n" 
   
end

-- @brief parses a C-like term statement and convert it into a LUA statement
function parse_term(term, caller)

 -- only make the statements conform to lua syntax
 -- ex: replace != by ~=  
  local lua_term 
  local matched = false 
 
  s,e = string.find(term,"max%(")
  if(s ~= nil and e ~= nil) then
    lua_term =  term
    matched = true
  end

  s,e = string.find(term,"min%(")
  if(s ~= nil and e ~= nil) then
    lua_term = term
    matched = true
  end

  
  s,e = string.find(term,"%+%+")
  if(s ~= nil and e ~= nil) then
    lua_term = nil
    matched = true
  end

  s,e = string.find(term,"%+%=")
  if(s ~= nil and e ~= nil) then
    if (caller == "FOR_TERM_3") then
      lua_term = string.sub(term, e+1, -1)    
    else
      lua_term = term
    end
    matched = true
  end

  s,e = string.find(term,"[><][=]")
  if(s ~= nil and e ~= nil) then      
    if(caller == "FOR_TERM_2") then
      lua_term = string.sub(term,e+1,-1) 
    else
      lua_term = term
    end 
    matched = true
  end
 
  if not matched then
    lua_term = term
  end


  return lua_term 

end


 
