#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/stat.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "atpquery.h"


extern void ATP_ERROR(const char *msg, ...);



void ATP_lua_error(lua_State *L, char *msg)
{
   ATP_ERROR("LUA script error at %s: %s\n",msg,lua_tostring(L,-1));
}



void oc_request_range_generation(atp_query_t *query)
{
   int status = 0;
   lua_State *L;
   L = luaL_newstate(); 
   if (L == NULL) {
     ATP_TRACE("error creating lua state");
     return;
   }

   luaL_openlibs(L); /* load lua libraries */
   const char *lua_path = getenv("ATP_PATH");
   char full_path[200];
   ATP_TRACE("querying OC with: %s\n",query->query_string);
   sprintf(full_path,"%s/lua/oc_query.lua",lua_path); 

   if (luaL_loadfile(L, full_path)) {
     ATP_ERROR("failed to load LUA script file %s\n",full_path);
     query->status = aq_lua_error;
     ATP_lua_error(L, "LUA loadfile");
     return;
   }

   ATP_TRACE("lua_pcall (init)\n");
   if(lua_pcall(L,0,0,0))
   {
      ATP_lua_error(L, "LUA pcall (init)");
      query->status = aq_lua_error;
      return;
   }
   lua_getglobal(L,"codegen");
   lua_pushstring(L,query->query_string);

   ATP_TRACE("lua_pcall (codegen)\n");
   if(lua_pcall(L,1,3,0)){
      ATP_lua_error(L, "codegen");
      query->status = aq_lua_error;
      return; 
   }
   int num_tuples = lua_tointeger(L,-1);

   ATP_TRACE("valid points gen: getting num_tuples: %d \n", num_tuples);
   if (num_tuples == -1)
   {
      query->status = aq_oc_error; 
      return;
   }
   lua_pop(L,1); //pop num_tuples from stack

   int tuple_num_elems = lua_tointeger(L,-1);
   ATP_TRACE("valid points gen: getting tuple_num_elems: %d \n", tuple_num_elems);
   lua_pop(L,1); //pop tuple_num_elems from stack

   int32_t *tuples = (int *)malloc(sizeof(int32_t *) * num_tuples * tuple_num_elems);
   
   int index = 0;
   lua_pushnil(L);  // Make sure lua_next starts at beginning  
   while (lua_next(L, -2))
   {
     int elem = 0;
     lua_pushnil(L);
     while(lua_next(L,-2))
     {
       tuples[(index * tuple_num_elems) + elem] = lua_tointeger(L,-1);
       ATP_TRACE("valid points gen: >> tuple[%d][%d] = %d\n", 
		 index, elem, tuples[(index * tuple_num_elems) + elem]);
       lua_pop(L,1);
       elem +=1;
     }
     lua_pop(L,1); // pop the tuple table
     index += 1;
   }
   lua_close(L);

   query->pointCombinations = tuples;
   query->numCombinations = num_tuples;
   query->combinationNumElems = tuple_num_elems;
   query->status = aq_range_success;
   return;
}



