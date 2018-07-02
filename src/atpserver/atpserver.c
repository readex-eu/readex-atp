
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <ctype.h>
#include "atpserver.h"
#include "atpconfig.h"

static int   read_cnt;
static char* read_ptr;
static char  read_buf[ 1000 ];

int
ATPS_init( const char* regHost, int regPort )
{
    uint8_t is_initialized = 0;
    static atp_server_t atps;
    atps.socket = 0;
    atps.port = 20000;
    atps.step = 10;
    atps.retries = 10; 
    atps.app_name = "ATPServer";
    if(regHost != NULL)
    {
        atps.registry_host= malloc(strlen(regHost)+1);
        strcpy (atps.registry_host, regHost);
        atps.registry_port = regPort;
    }
    if ( is_initialized == 0 )
    {
        /* Set the initialization flag to indicate that the adapter is initialized */
        is_initialized = 1;
        int atps_connection = ATPS_connection_connect(&atps);
        ATPS_receive_and_process_requests( atps_connection );
        fflush(stdout);
        return atps_connection;

    }
    return 0;
}

//---------------------------------------------------------------------------------------
int
ATPS_connection_connect( atp_server_t *s)
{
    static int8_t atps_is_connected = 0;
    if ( atps_is_connected )
    {
        return (1);
    }
    ATP_MESSAGE("starting ATP server version %d.%d.%d ...\n",
		ATP_VERSION_MAJOR, ATP_VERSION_MINOR, ATP_VERSION_PATCH);
    ATP_TRACE("git commit %s\n", ATP_GIT_COMMIT_HASH);
    int socket = ATPS_sockets_server_startup_retry( &(s->port), s->retries, s->step);
    if ( socket == -1 )
    {
        _Exit( EXIT_FAILURE );
    }
    atps_is_connected = 1;
    if(s->registry_host != NULL)
        ATPS_sockets_register_with_registry( s->port, 
                                             s->registry_port, 
                                             s->registry_host, 
                                             s->app_name );
    ATP_TRACE("PREPARING client connection ...\n");
    socket = ATPS_sockets_server_accept_client( socket );
    return socket;
}
//------------------------------------------------------------------------------------------


int
ATPS_sockets_server_startup_retry( uint64_t* initPort,
                                        int  retries,
                                        int  step )
{
    int                sock;
    int                yes  = 1;
    int                stat = -1;
    int                port;
    struct sockaddr_in my_addr;                 /* my address information */

    /**
     * create a new socket socket() returns positive integer on success
     */

    for ( port = ( int )*initPort; port <= *initPort + retries * step && stat == -1; port = port + step )
    {
        stat = 0;

        if ( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
        {
            if ( port + step > *initPort + retries * step )
            {
               ATP_ERROR("socket_server_startup::socket()\n" );
            }
            stat = -1;
        }
        else
        {
            if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) ) < 0 )
            {
                if ( port + step > *initPort + retries * step )
                {
                    ATP_ERROR("socket/ATP_server_startup::setsockopt()\n" );
                }
                stat = -1;
            }
            else
            {
                my_addr.sin_family      = AF_INET;                        /** host byte order */
                my_addr.sin_port        = htons( port );                  /** short, network byte order */
                my_addr.sin_addr.s_addr = INADDR_ANY;                     /** automatically fill with my IP */
                memset( &( my_addr.sin_zero ), '\0', 8 );                 /** zero the rest of the struct */

                if ( bind( sock, ( struct sockaddr* )&my_addr, sizeof( struct sockaddr ) ) < 0 )
                {
                    if ( port + step > *initPort + retries * step )
                    {
                        ATP_ERROR("socket_server_startup::bind()\n" );
                    }
                    stat = -1;
                }
                else
                {
                    if ( listen( sock, 1 ) < 0 )
                    {
                        if ( port + step > *initPort + retries * step )
                        {
                            ATP_ERROR("socket_server_startup::listen()\n" );
                        }
                        stat = -1;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }

    if ( stat == -1 )
    {
        return -1;
    }
    else
    {
        ATP_TRACE ("Exiting %s with success, port = %d\n", __func__, port);
        *initPort = ( uint64_t )port;
        return sock;
    }
}
//-------------------------------------------------------------------------

void
ATPS_sockets_register_with_registry( uint64_t port,
                                     uint64_t regPort,
                                     char*    regHost,
                                     char*    appName )
{
    registry* reg;
    int       i;
    int       nprocs, rank, initialized;
    struct p_info
    {
        char hostname[ 100 ];
        int  port;
        int  cpu;
        int  rank;
    };

    typedef struct p_info P_info;
    P_info myinfo;
    int    entry_id;
    char   appl_name[ 2000 ];
    char   psc_reghost[ 200 ];
    int    psc_regport;
    char   site_name[ 50 ];
    char   machine_name[ 50 ];
    char   library_name[ 50 ];

    sprintf( appl_name, "%s", appName );
    sprintf( psc_reghost, "%s", regHost );
    //UTILS_IO_GetHostname( psc_reghost, 100 );
    psc_regport = ( int )regPort;
    sprintf( site_name, "none" );
    sprintf( machine_name, "none" );
    sprintf( library_name, "ATPSERVER" );
    myinfo.cpu  = 1;                                                                                    /// get rid of later
    myinfo.port = ( int )port;
    myinfo.rank = 1;
    //UTILS_IO_GetHostname( myinfo.hostname, 100 );

    reg = sockets_open_registry( psc_reghost, psc_regport );

    if ( !reg )
    {
        ATP_ERROR("Cannot open registry at %s:%d\n", psc_reghost, psc_regport );
        exit( 1 );
    }
    entry_id = sockets_registry_create_entry( reg,
                                              appl_name,
                                              site_name,
                                              machine_name,
                                              psc_reghost, //TODO myinfo.hostname,
                                              myinfo.port,
                                              myinfo.rank,
                                              library_name,
                                              "none" );
    if ( entry_id == 0 )
    {
        ATP_ERROR( "Failed to create registry entry\n" );
    }
    else
    {
        ATP_TRACE( "Entry_id= %d\n", entry_id );
    }
    if ( !sockets_close_registry( reg ) )
    {
        ATP_TRACE("Something went wrong when closing registry\n" );
    }

}

int
sockets_registry_create_entry( registry*   reg,
                               const char* app,
                               const char* site,
                               const char* mach,
                               const char* node,
                               int         port,
                               int         pid,
                               const char* comp,
                               const char* tag )
{
    int  id = 0;
    char buf[ BUFSIZE ];
    int  n = 0;
    sprintf( buf, "%s "
             "app=\"%s\" "
             "site=\"%s\" "
             "mach=\"%s\" "
             "node=\"%s\" "
             "port=%d "
             "pid=%d "
             "comp=\"%s\" "
             "tag=\"%s\"\n",
             CMD_CREATE, app, site, mach, node, port, pid, comp, tag );
    ATP_sockets_write_line( reg->sock_, buf );
    ATP_sockets_read_line( reg->sock_, buf, BUFSIZE );
    n = sscanf( buf, MSG_CREATE_SUCCESS, &id );
    if ( n < 1 )
    {
        return 0;
    }

    return id;
}

/*
 * open connection to the registry server,
 * initialize data-structures
 */
registry*
sockets_open_registry( const char* hostname,
                       int         port )
{
    registry* reg = ( registry* )malloc( sizeof( registry ) );
    char buf[ BUFSIZE ];

    reg->hostname_= malloc(strlen(hostname)+1);

    strcpy(reg->hostname_, hostname);
    reg->port_     = port;
    reg->sock_ = sockets_client_connect_retry( reg->hostname_, reg->port_, 10 );

    if ( reg->sock_ < 0 )
    {
        free( reg->hostname_ );
        free( reg );
        return 0;
    }

    ATP_sockets_read_line( reg->sock_, buf, BUFSIZE );

    if ( strncmp( buf, PREFIX_SUCCESS, strlen( PREFIX_SUCCESS ) ) )
    {
        /** something went wrong */
        close( reg->sock_ );
        free( reg->hostname_ );
        free( reg );
        return 0;
    }

    return reg;
}

int
sockets_client_connect_retry( char* hostname,
                              int   port,
                              int   retries )
{
    struct addrinfo  hints;
    struct addrinfo* result;
    int              s, sock;
    int              success, i;

    if ( port >= 999999 )
    {
        ATP_WARNING( "Port number %d is too big\n", port );
        return -1;
    }

    char* port_s = ( char* )malloc( 6 * sizeof( char ) );
    sprintf( port_s, "%d", port );

    success = -1;
    for ( i = 0; i < retries && success == -1; i++ )
    {
        sleep( 4 );
        success = 0;

        memset( &hints, 0, sizeof( struct addrinfo ) );
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags    = 0;
        hints.ai_protocol = 0;              /* Any protocol */

        s = getaddrinfo( hostname, port_s, &hints, &result );
        if ( s != 0 )
        {
            if ( i == retries - 1 )
            {
                ATP_WARNING( "Could not get address info for %s:%d\n", hostname, port );
            }
            success = -1;
            //freeaddrinfo( result );
            continue;
        }

        sock = socket( result->ai_family, result->ai_socktype, result->ai_protocol );

        if ( sock == -1 )
        {
            if ( i == retries - 1 )
            {
                ATP_WARNING( "Could not create socket %s:%d\n", hostname, port );
            }
            success = -1;
            freeaddrinfo( result );
            continue;
        }

        if ( connect( sock, result->ai_addr, result->ai_addrlen ) == -1 )
        {
            if ( i == retries - 1 )
            {
                ATP_WARNING( "Could not connect to %s:%d\n", hostname, port );
            }
            success = -1;
            freeaddrinfo( result );
            continue;
        }
        freeaddrinfo( result );
    }

    free( port_s );

    if ( success == -1 )
    {
        sock = -1;
    }
    return sock;
}


/*
 * close the connection to the registry server
 * returns 1 on success, 0 on failure
 */
int
sockets_close_registry( registry* reg )
{
    int  ret;
    char buf[ BUFSIZE ];

    sprintf( buf, "%s\n", CMD_QUIT );
    ATP_sockets_write_line( reg->sock_, buf );

    ATP_sockets_read_line( reg->sock_, buf, BUFSIZE );
    if ( strncmp( buf, PREFIX_SUCCESS, strlen( PREFIX_SUCCESS ) ) )
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    close( reg->sock_ );
    //free( reg->hostname_ );
    free( reg );
    return ret;
}

void
ATP_sockets_write_line( int         sock,
                              const char* str )
{
    if ( write( sock, str, strlen( str ) ) == -1 )
    {
        ATP_WARNING( "Could not write to socket!\n" );
    }
}

inline void
ATP_sockets_write_data( int         sock,
                              const void* buf,
                              int         nbyte )
{
    if ( write( sock, buf, nbyte ) == -1 )
    {
        ATP_WARNING( "Could not write to socket!\n" );
    }
}

static inline int
sockets_socket_my_read( int   fd,
                        char* ptr )
{
    if ( read_cnt <= 0 )
    {
        again:
        if ( ( read_cnt = read( fd, read_buf, sizeof( read_buf ) ) ) < 0 )
        {
            if ( errno == EINTR )
            {
                goto again;
            }
            return -1;
        }
        else
        if ( read_cnt == 0 )
        {
            return 0;
        }
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

int
ATP_sockets_read_line( int   sock,
                       char* str,
                       int   maxLen )
{
    int  n, rc;
    char c, * ptr;

    ptr = str;
    for ( n = 1; n < maxLen; n++ )
    {
        if ( ( rc = sockets_socket_my_read( sock, &c ) ) == 1 )
        {
            if ( c  == '\n' )
            {
                break;                          /* newline is stored, like fgets() */
            }
            *ptr++ = c;
        }
        else
        if ( rc == 0 )
        {
            *ptr = 0;
            return n - 1;                           /* EOF, n - 1 bytes were read */
        }
        else
        {
            return -1;                              /* error, errno set by read() */
        }
    }

    *ptr = 0;          /* null terminate like fgets() */

    return n;
}

//----------------------------------------------------------------------
int
ATPS_sockets_server_accept_client( int sock )
{
    int newsock;

    struct sockaddr_in client_addr;       /* client's address information */

    unsigned int sin_size;

    sin_size = sizeof( struct sockaddr_in );
    ATP_TRACE( "Waiting for client to connect...\n" );
    if ( ( newsock = accept( sock, ( struct sockaddr* )&client_addr, &sin_size ) ) < 0 )
    {
        ATP_ERROR( "socket_server_accept_client::accept() error\n" );
        return -1;
    }
    ATP_TRACE("client connected \n");
    return newsock;
}
//-----------------------------------------------------------------------

void
ATPS_receive_and_process_requests( int connection )
{

    int error =  0;
    socklen_t len = sizeof(error);
    int retval = 0;
    while ((retval = getsockopt(connection,SOL_SOCKET, SO_ERROR, &error, &len))==0)
    {
        char buffer[ 2000 ];
        memset( buffer, '\0', 2000 );
        int length;

        while((length = recv(connection, buffer, 2000, 0)) < 0) 
        {
        }
        int skip_expressions = 0;
        int end_reached = 0;
        int i;
        for ( i = 0; i < length; i++ )
        {
            if(end_reached)
            {
                buffer[i] = '\0';
            }
            if(buffer[ i ] == ';')
            {
               end_reached = 1;
            }
            buffer[ i ] =  buffer[ i ] ;

        }
        
        ATP_TRACE("Received from socket: %s\n", buffer );
        ATPS_dispatch_and_respond(buffer,connection);
	
    }

}
//-------------------------------------------------------------------------

void ATPS_send_error(int connection)
{
    int32_t err = -1;
    if(send(connection, &err, sizeof(int32_t), 0) < 0)
    {
        ATP_ERROR("could not send message over socket in ATPS_send_error()\n" );
    }
}

void ATPS_send_atp_specs(int connection)
{
      
    int j;
    int32_t domain_nums = ATP_get_num_domains();
    int32_t param_nums = ATP_get_total_parameter_number();
    int32_t param_values_nums = ATP_get_total_parameter_values_number();
    ATP_TRACE("sending num domains: %d, param nums: %d, and param values num: %d\n", 
	      domain_nums, param_nums, param_values_nums);
    int32_t param_nums_details[3] = {domain_nums, param_nums, param_values_nums};

    if(send(connection, param_nums_details, (sizeof(int32_t) * 3), 0) < 0)
    {
        ATP_ERROR("could not send message over socket in ATPS_send_atp_specs()\n" );
    }

    int32_t *domain_details = ATP_get_total_domain_details();
    for (j = 0; j < domain_nums; j++)
    {
      ATP_TRACE("sending num of parameters per domain: %d\n", domain_details[j]);
    }

    if (send(connection, domain_details, domain_nums * sizeof(int32_t), 0) < 0)
    {
      ATP_ERROR("could not send message over socket in ATPS_send_atp_specs()\n");
    }

    char *domain_names = ATP_get_total_domain_names();
    for (j = 0; j < domain_nums; j++)
    {
      ATP_TRACE("sending domain names: %s\n", &(domain_names[j*100]));
    }

    if (send(connection, domain_names, (domain_nums * 100), 0) < 0)
    {
      ATP_ERROR("could not send message over socket in ATPS_send_atp_specs()\n");
    }

    char *param_names = ATP_get_total_parameter_names();
    for (j = 0; j < param_nums; j++)
    {
      ATP_TRACE("sending names: %s\n", &(param_names[j*100]));
    } 

    if(send(connection, param_names, (param_nums * 100), 0) < 0)
    {
        ATP_ERROR("could not send message over socket in ATPS_send_atp_specs()\n" );
    }

    int32_t *param_details = ATP_get_total_parameter_details();

    ATP_TRACE("sending param details:\n");
    int i, index = 0;
    for(i=0; i<(param_nums); i++)
    {
         ATP_TRACE("PARAM: %d\n",i);
         ATP_TRACE("-> type [%d][%d] = %d\n",i, 0, param_details[index]);
         ATP_TRACE("-> default [%d][%d] = %d\n",i, 1, param_details[index+1]);
         ATP_TRACE("-> max values [%d][%d] = %d\n",i, 2, param_details[index+2]);

         int values_nums = (param_details[index] == 1)? 3 : param_details[index+2] ;
         int k;
         ATP_TRACE("--> number of values in values tuple %d\n",values_nums);
         for(k=0; k< values_nums; k++)
         {
             ATP_TRACE("--> [%d][%d] = %d\n",i, (3 + k), param_details[index+k+3]);
         }
         index = index + 3 + values_nums;
    }
    if(send(connection, param_details, 
           (((param_nums * 3) + param_values_nums) * sizeof(int32_t)), 0)<0)
    {
        ATP_ERROR("could not send message over socket in ATPS_send_atp_specs()\n" );
    }

    free(domain_details);
    free(domain_names);
    free(param_names);
    free(param_details);
}



void 
ATPS_send_valid_parameters(int connection, char *domName)
{
   atp_query_t *query = malloc(sizeof(atp_query_t));

   if(ATP_domain_find(domName) == NULL)
   {
      query->status = aq_domain_not_found;
      ATP_ERROR("PTF request error >> parameter domain name not found %s\n",domName);
      ATP_TRACE("PTF request error >> parameter domain name not found %s\n",domName);
   }else{
       query->query_string = ATP_build_oc_request(domName);
       oc_request_range_generation(query);
   }

   if(query->status == aq_range_success)
   {
      ATP_TRACE("Oc request succeeded, sending data\n");
      int32_t info_array[3] = {1, query->numCombinations, query->combinationNumElems};
      ATP_TRACE("Sending info: %d, num combinations: %d, elements per combination: %d\n",
		info_array[0], info_array[1], info_array[2]);
      if(send(connection, info_array, (3 * sizeof(int32_t)),0) < 0)
        
      {
          ATP_ERROR("could not send message over socket in ATPS_send_valid_parameters()\n" );
      }
      ATP_TRACE("Sending combinations: ");
      int elem;
      for (elem = 0; elem < query->numCombinations * query->combinationNumElems; elem++)
	ATP_PRINT("%d ", query->pointCombinations[elem]);
      ATP_PRINT("\n");

      if(send(connection,
         query->pointCombinations,
         (query->numCombinations * query->combinationNumElems * sizeof(int32_t)), 0)<0)
      {
          ATP_ERROR("could not send message over socket in ATPS_send_valid_parameters()\n" );
      }
   }else{
      ATP_TRACE("Oc request failed, code %d\n",query->status);
      int32_t info_array[3] = {0,0,0}; 
      if(send(connection, info_array, (3 * sizeof(int32_t)), 0) < 0)
        
      {
          ATP_ERROR("could not send message over socket in ATPS_send_valid_parameters()\n" );
      }
     
   }
   free(query);
}



char *
extract_domain_name(const char *request)
{
    char *dup = strdup(request);
    char *param = NULL;
    if(dup == NULL)
    {
        ATP_ERROR("could not allocate memory to retrieve request parameter 'extract_domain_name'\n");
    }
    char cursor = dup[0];
    int index = 0;
    while(cursor != '\0' && cursor != ',' )
    {
      index++;
      cursor = dup[index];   
    }

    if(cursor == '\0')
        return NULL;
 
    dup[index] = '\0';
    index++;
    param = &dup[index];   
    cursor = *param;
    while(cursor != ';')
    {
        index++;
        cursor = dup[index];
    }
    dup[index] = '\0';   

    return param;
}


void
ATPS_dispatch_and_respond(const char *request, int connection)
{

   char *response = NULL;

   if(!load_desc_file())  //check if the ATP desc file is present and load its content  
      ATPS_send_error(connection);

   char request_name[2][20];
   strcpy(request_name[0],"getatpspecs;");
   strcpy(request_name[1],"getvalidparams");

   if(strcmp(request,request_name[0]) == 0)  //query: list of domains with their parameters
   {
       ATP_TRACE("request matched as 'GETATPSPECS'\n");
       ATPS_send_atp_specs(connection); 
   }else if(strncmp(request,request_name[1],14) == 0)//query: valid parameters 
   {
       char *domName = extract_domain_name(request);
       ATP_TRACE("request matched as 'GETVALIDPARAMS' with domnain name: %s\n",domName);
       ATPS_send_valid_parameters(connection,domName); 
   }else{
       ATP_TRACE("request not matched sending error \n");
       ATPS_send_error(connection);
   }
   
}




