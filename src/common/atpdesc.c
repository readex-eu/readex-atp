
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/stat.h>
#include "json.h"
#include "atpdesc.h"
#include "rrl/user_parameters.h"  // RRL needs to be linked to the atplib at compile time


//extern void ATP_ERROR(const char *msg, ...);

 
void data_buf_init(data_container_t *data_buf)
{
  data_buf->current_domain = NULL; 
  data_buf->current_param = NULL;
  data_buf->current_constraint = NULL;
  
  data_buf->pname = NULL;
  data_buf->cname = NULL;
  data_buf->range = NULL;
  data_buf->nvalues = 0;
  data_buf->explorations = NULL;
  data_buf->nexplorations = 0; 
  data_buf->cursor = 0;
  data_buf->type = 0;
  data_buf->dflt = 0;
  data_buf->expr = NULL; 
  data_buf->in_domain = 0;
  data_buf->in_param = 0; 
  data_buf->in_range = 0; 
  data_buf->in_type = 0; 
  data_buf->in_default = 0; 
  data_buf->in_constraint = 0; 
  data_buf->in_exploration = 0; 
  data_buf->param_counter = 0;
  data_buf->constraint_counter = 0;
}
                    
void commit_param(data_container_t *data_buf)
{
    if(data_buf->param_counter)
    {
         data_buf->current_param = ATP_param_new(data_buf->pname,
                       data_buf->type, data_buf->dflt,data_buf->current_domain);
         ATP_param_add_values(data_buf->current_param,data_buf->range,
                              data_buf->nvalues);               
	 rrl_atp_param_declare(data_buf->pname, data_buf->dflt, 
			       data_buf->current_domain->domName);
	 ATP_TRACE("Load parameter %s in domain %s from ATP description file\n",
		   data_buf->pname, data_buf->current_domain->domName);
    }
}

void commit_constraint(data_container_t *data_buf)
{
    if(data_buf->constraint_counter)
    {
        ATP_constraint_new(data_buf->cname, data_buf->expr, 
                              data_buf->current_domain);
    }
}

void enter_section(data_container_t *data_buf,json_value *value, int rank)
{
    data_buf->in_param = 0;
    data_buf->in_constraint = 0;
    data_buf->in_exploration = 0;
    if(strcmp(value->u.object.values[rank].name,"params") == 0)
    {
        data_buf->in_param = 1;
    } 
    if(strcmp(value->u.object.values[rank].name,"constraints") == 0)
    {
        data_buf->in_constraint = 1;
    }
    if(strcmp(value->u.object.values[rank].name,"explorations") == 0)
    {
        data_buf->in_exploration = 1;
    }
}

void enter_subsection(data_container_t *data_buf, json_value *value, int rank)
{
    data_buf->in_range = 0;    
    data_buf->in_type = 0;    
    data_buf->in_default = 0;    
    if(strcmp(value->u.object.values[rank].name,"range") == 0)
    {
         data_buf->in_range = 1;    
    }
    if(strcmp(value->u.object.values[rank].name,"type") == 0)
    {
         data_buf->in_type = 1;    
    }
    if(strcmp(value->u.object.values[rank].name,"default") == 0)
    {
        data_buf->in_default = 1;    
    }
}
 
void process_object(data_container_t *data_buf, json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.object.length;
        for (x = 0; x < length; x++) 
        {   
            if(depth == 1)
            {
                if(data_buf->in_domain == 1)
                {
                    commit_param(data_buf);
                    commit_constraint(data_buf);
                }
                data_buf_init(data_buf);
                data_buf->in_domain = 1;
                data_buf->current_domain = ATP_domain_new
                                            (value->u.object.values[x].name);
                 
            }
 
            if(depth == 2)
            { 
                 enter_section(data_buf, value, x);
            }
            if(depth == 3)
            {
               if(data_buf->in_param == 1)
	       {
                   commit_param(data_buf); 
                   data_buf->param_counter = data_buf->param_counter + 1; 
                   data_buf->pname = value->u.object.values[x].name; 
               }
               if(data_buf->in_constraint == 1)
               {
                   commit_constraint(data_buf); 
                   data_buf->constraint_counter = data_buf->constraint_counter + 1; 
                   data_buf->cname = value->u.object.values[x].name; 
               }
               
            } 
            if(depth == 4)
            { 
                 enter_subsection(data_buf, value, x);
            } 
            process_value(data_buf, value->u.object.values[x].value, depth);
        }
        if((data_buf->in_domain == 1) && (depth == 1) && (length > 0))
        { 
            commit_param(data_buf);
            commit_constraint(data_buf);
        }
}


void process_array(data_container_t *data_buf, json_value* value, int depth)
{
    int length, x;
    if (value == NULL) {
        return;
    }
    length = value->u.array.length;
    if(data_buf->in_range == 1)
    {
        data_buf->nvalues = length;
        data_buf->range = (int32_t *)malloc(sizeof(int32_t) * length);
        data_buf->cursor = 0;
    }
    if(data_buf->in_exploration == 1)
    {
        data_buf->nexplorations = length;  
        data_buf->explorations = malloc(sizeof(int32_t) * length);
        data_buf->cursor = 0;
    }    
    for (x = 0; x < length; x++) {
        process_value(data_buf, value->u.array.values[x], depth);
    }
}


void process_value(data_container_t *data_buf, json_value* value, int depth)
{
        if (value == NULL) {
                return;
        }
        switch (value->type) {
            case json_none:
                break;
            case json_object:
                process_object(data_buf, value, depth+1);
                break;
            case json_array:
                process_array(data_buf, value, depth+1);
                break;
            case json_integer:
                if(data_buf->in_exploration == 1)
                   data_buf->explorations[data_buf->cursor++] = value->u.integer;
                if(data_buf->in_param == 1)
                {
                    if(data_buf->in_range == 1)
                       data_buf->range[data_buf->cursor++] = value->u.integer;
                    if(data_buf->in_type == 1)
                       data_buf->type = value->u.integer;
                    if(data_buf->in_default == 1)
                       data_buf->dflt = value->u.integer;
                }     
                break;
            case json_string:
                if(data_buf->in_constraint == 1)
                {
                   data_buf->expr = value->u.string.ptr;
                }
                break;
	    default:
		break;
        }
}


int parse_atp_desc(const char *filename)
{


    FILE *fp = NULL; 
    struct stat filestatus;
    int file_size;
    char *file_contents;
    json_char *json;
    json_value *value;
     if ( stat(filename, &filestatus) != 0) 
     {
         ATP_ERROR("in parse_atp_desc(), file %s not found\n", filename);
         return 0;
     }
     file_size = filestatus.st_size;
     file_contents = (char*)malloc(filestatus.st_size);
     if ( file_contents == NULL) {
         ATP_ERROR("in parse_atp_desc(), unable to allocate memory\n");
         return 0;
     }

     fp = fopen(filename, "rt");
     if (fp == NULL) {
          ATP_ERROR("in parse_atp_desc(), unable to open file %s\n",filename);
          fclose(fp);
          free(file_contents);
          return 0;
     }
     if ( fread(file_contents, file_size, 1, fp) != 1 ) {
          ATP_ERROR("in parse_atp_desc(), unable to read content of file %s\n",filename);
          fclose(fp);
          free(file_contents);
          return 0;
     }
     fclose(fp);

     json = (json_char*)file_contents;
     value = json_parse(json,file_size);
     if (value == NULL) {
         ATP_ERROR("in parse_atp_desc(), unable to parse data\n");
         free(file_contents);
         return 0;
     }
     data_container_t data_buf;
     ATP_init();
     process_value(&data_buf, value, 0);
     json_value_free(value);
     free(file_contents);
     return 1;   
}

int load_desc_file()
{

    static int desc_file_loaded = 0;
    if(!desc_file_loaded)
    {
        ATP_TRACE("Loading ATP description file\n");

	char file_path[200];
	const char *atp_path = getenv("ATP_DESCRIPTION_FILE");
	if (atp_path == NULL)
	  sprintf(file_path,"ATP_description_file.json");
	else
	  sprintf(file_path,"%s",atp_path);

        int status = parse_atp_desc(file_path);

        if(status == 0) //something wrong in desc file parsing
        {
        
	  ATP_TRACE("Error while loading %s \n",file_path);
           return (0);
        }
        ATP_TRACE("%s Loaded\n",file_path);
        desc_file_loaded = 1;
    }

    return (1);      
}

