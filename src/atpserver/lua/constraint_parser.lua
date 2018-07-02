#!/bin/lua



--[[
this file contains:
 - a LUA implementation of a relational expressions parser and validator.

Author: Z.Bendifallah (zakaria.bendifallah@intel.com)
--]]


--[[
 Definition of the expression parser

Accepted tokens are:
NUmeric             | [1-9]+
Alphanumeric names  | [a-z 0-9]+
comparison operators| <,<=,>,>=,=,!=
arithmetic operators| +,-,*
parenthesis         | (,)
logical operators   | &&,||

--]]


 local Parser = { code = "",
            cursor = "",
            cToken = "",
            cTokenType="",
            index = 1,
            typePatterns= {N= "^[%d]+", A="^[%w]+",C="^[><=][=]?",R="^[%+%-%*]",P="^[%(%)]",L="^[&|]"},
            types = {"N", "A","C","R","P","L","U"} -- num,alphanum,comparison,arith,parenth,logic,unknown
         }



 function Parser:new(str)     
  self.code =str
  self.cursor = str
  self.cToken = ""
  self.cTokenType = ""
  self.index = 1
  
 end

 -- parses the next token (spaces ignored)
 function Parser:next_token()

  local tokenAvail = 0
 
  local s,e = string.find(self.cursor,"^%s+")
  if s ~= nil and e ~= nil then
    self.cursor = string.sub(self.cursor,e+1,-1)
  end

  local match = false
  local token = nil
  for k,v in pairs(self.typePatterns) do 
    s,e = string.find(self.cursor, v)
    --print("searching " .. v .. ", found:")
    --print(s,e)
    if s ~= nil and e ~= nil then
      self.cToken = string.sub(self.cursor,s,e)
      self.cursor = string.sub(self.cursor,e+1,-1) 
      self.cTokenType = k
      match = true
      token = self.cToken
      tokenAvail = 1
      print("token fetched: " .. self.cToken)
      break
    end 
  end
  if not match then
    s,e = string.find(self.cursor, "[^%s]+")
    if s ~= nil then 
      print("token type not recognized: " .. string.sub(self.cursor,s,e)) 
      self.cToken = string.sub(self.cursor,s,e)  
      self.cTokentype = "U"
      tokenAvail = 2
    else
      --print("end of code reached")   
      self.cToken = ""
      self.cTokentype = "U"
      tokenAvail = 0
    end
  end

  return tokenAvail
 end

 -- returns the last parsed token (spaces ignored)
 function Parser:get_token()
  return self.cToken 
 end

 function Parser:get_token_type()
  return self.cTokenType
 end

--[[
 End of definition of the expression parser
--]]


--[[
 definition of the syntax tree generator
--]]

 recParser = {
   --node = {parent = nil, value = nil, leftCh = nil, rightCh = nil}, 
   parser = nil,
   
   status  -- 0 no parsing , 1: parse successful, 2: parse failed 
 }   

 function recParser:new_parser(parser)
  self.parser = parser
  self.status = 0
 end

 function recParser:parse_code(code)
   self.parser:new(code)
   self.status = 0
   self:recursive_descent() 
 end

 function recParser:expect(str)

   if self.parser:get_token() ~= str then
     print("expected token: ",str)
     self.status = 2
     return false
   end
   print("token matched: ", str)
   self.parser:next_token()
   return true
 end

 function recParser:accept(str)

   if self.parser:get_token() ~= str then
     return false
   end
   print("token matched: ", str)
   self.parser:next_token()
   return true
 end

 function recParser:accept_type(str)

   if self.parser:get_token_type() ~= str then
     return false
   end
   print("token type matched: ", str)
   self.parser:next_token()
   return true
 end

 function recParser:recursive_descent()

   self.parser:next_token()  
   self:logical_expr()
   if self.status ~= 2 then
     self.status = 1
     print("Code parsing succeeded")
   else
     print("Code parsing failed")
   end
 end

 function recParser:logical_expr()

   self:boolean_expr()
  
   if self:accept("|") then
      self:boolean_expr()
   end      
 end

 function recParser:boolean_expr()

    self:eq_expr()
     
   if self:accept("&") then
      self:eq_exp()
   end      
 end

 function recParser:eq_expr()

    self:rel_expr()
     
    if self:accept("=") or self:accept("!=") then
      self:eq_exp()
    end      
 end

 function recParser:rel_expr()

    self:add_expr()
     
    if self:accept(">") or self:accept(">=") or self:accept("<") or 
       self:accept("<=") then
      self:add_expr()
    end      
 end

 function recParser:add_expr()

    self:mul_expr()
     
    if self:accept("+") or self:accept("-") then
      self:mul_expr()
    end      
 end

 function recParser:mul_expr()

    self:unary_expr()
     
    if self:accept("*") then
      self:unary_exp()
    end      
 end

 function recParser:unary_expr()
   
    if self:accept("!") or self:accept("-") then
    end      
    self:prime_expr()
 end

 function recParser:prime_expr()
   
   if self:accept("(") then
     self:logical_expr()
     self:expect(")")
   
   elseif self:accept_type("N")  then 
     self:numeric_ref() 
   elseif self:accept_type("A") then
     self:param_ref() 
   else 
     print("expected one ef tokens: (, <number>, <ident> ")
     self.status = 2 
   end
 end 

 function recParser:numeric_ref()
   print("numeric matched")                
 end
 
 function recParser:param_ref()
   print("identifier matched")                
 end

--[[
 End of definition of the syntax tree generator
--]]


 myp = recParser or {}
 myp:new_parser(Parser)
 --myp:parse_code("(ab > cd) > 6)")
 return myp    

 --while (myp:next_token())do
 --  print("token :" .. myp:get_token() .. "|" .. myp:get_token_type())
 --end


