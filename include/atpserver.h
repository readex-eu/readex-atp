/** 
* @file atpserver.h
* 
* @author Zakaria Bendifallah (zakaria.bendifallah@intel.com)
* 
* @brief This file contains the code to connect to the PTF server as well as the code
* to receive requests and prepare responses to them      
*
* ATPserver consists of functions to connect to PTF and to register to its registry 
* service. After connection the ATPserver listens for PTF requests regarding ATP 
* parameter details and responds to them. 
*
* The ATPserver is completely independent from the ATPlibrary, it however loads the
* content of the ATP description file exported by the library. The file location can
* be set by environment variable ATP_DESCRIPTION_FILE, otherwise ATP_description_file.json
* from cwd will be loaded
*
* @date 01/04/2017
**/


#ifndef __ATPSERVER__
#define __ATPSERVER__


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

#include "atpquery.h"
#include "ATP_registry_protocol.h"

//uint64_t atp_server_port = 50910;
#define BUFSIZE 512

extern void ATP_ERROR(const char *msg, ...);


typedef struct atp_server_t{
  /*
    scorep_oa_port  --> atps_port   

  */

  char *app_name;
  char *registry_host;
  int socket;
  uint32_t retries;
  uint32_t step;
  uint64_t port;
  uint64_t registry_port;

} atp_server_t;

typedef struct
{
    char* hostname_;
    int   port_;
    int   sock_;
} registry;

int
ATPS_init(const char* regHost, int regPort);

int
ATPS_connection_connect( atp_server_t *s);


int
ATPS_sockets_server_startup_retry( uint64_t* initPort,
                                        int       retries,
                                        int       step );
 
void
ATPS_sockets_register_with_registry( uint64_t port,
                                     uint64_t regPort,
                                     char*    regHost,
                                     char*    appName );
int
sockets_client_connect_retry( char* hostname,
                              int   port,
                              int   retries );

int
sockets_registry_create_entry( registry*   reg,
                               const char* app,
                               const char* site,
                               const char* mach,
                               const char* node,
                               int         port,
                               int         pid,
                               const char* comp,
                               const char* tag );

registry*
sockets_open_registry( const char* hostname,
                       int         port );

int
sockets_close_registry( registry* reg );

int
ATP_sockets_read_line( int   sock,
                             char* str,
                             int   maxLen );

void
ATP_sockets_write_line( int         sock,
                              const char* str );

int
ATPS_sockets_server_accept_client( int sock );

//@brief receives a request from the ATP server and call the function 
//in charge of dispatching it
void
ATPS_receive_and_process_requests( int connection );

//@brief dispatches a PTF request and calls the approprtiate functions
//to prepare the resposnes     
void
ATPS_dispatch_and_respond(const char *msg, int connection);

//@brief sends and error message as a response to a request, in case
//soemthing wrong happened
void 
ATPS_send_error(int connection);

//@brief responds to PTF request "getatpspecs" by sending details on the
//parameters  
void 
ATPS_send_atp_specs(int connection);

//@brief responds to PTF request "getvalidparams,DOMAIN_NAME" by sending
//an array of the valid tuples of values for the parameters contained
// the specified domain 
void 
ATPS_send_valid_parameters(int connection, char *domName);
#endif



