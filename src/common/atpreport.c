#include "atpreport.h"

static int first_write = 1;

void ATP_ERROR(const char *msg, ...)
{

    const char *atp_path = getenv("ATP_PATH"); 
    char file_path[200];
    uint32_t len = strlen(msg);
    char full_msg[len + 50];
    strcpy(full_msg,"ATP ERROR: ");
    strcat(full_msg,msg);
    va_list args;
    va_start(args, msg);
    if (atp_path == NULL)
    {
        vfprintf(stderr, full_msg, args);
        va_end(args);
        return;
    }
    //sprintf(file_path,"%s/%s", atp_path,ATP_REPORT_FILE);
    sprintf(file_path,"%s",ATP_REPORT_FILE);
    FILE *fp;
    if (first_write)
    {
      fp = fopen(file_path,"w");
      first_write = 0;
    }else{ 
      fp = fopen(file_path,"a");
    }
    if (fp == NULL)
    {
        fprintf(stderr," could not open report file for writing\n");
        vfprintf(stderr, full_msg, args);
    }else{
        vfprintf(fp, full_msg, args);
        fclose(fp);
    }
    va_end(args);
}
