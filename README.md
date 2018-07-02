# READEX Application Tuning Parameters component

The READEX ATP component is written as part of the READEX project
(www.readex.eu)

## Compilation and Installation

### Prerequisites

Are needed for compilation:

C compiler (GCC/ICC)

LUA interpreter and libraries (tested only with version >=5.1)


### Building and installation

Two components are available:
* ATP library: libatp.so 
* ATP server : atpserver

In ATP root directory type:

    mkdir build
    cd build
    cmake ..

Optional cmake options
    add -DCMAKE_INSTALL_PREFIX=aptlib-install-directpry to change the path where ATP will be installed
    add -DCMAKE_BUILD_TYPE=Debug to enable ATP Debug output

Second invoke the make commands:

    make
    make install


The libatp.so would be located by default in `lib`.
The atpserver binary would be located by default in `bin`.

## Documentation
TODO

## Usage

The ATP component holds a library `libatp.so` and an executable `atpserver`: 
`libatp.so` needs to linked to the target application as a shared library
`atpserver` binary should be launched by the PTF framewok and responds
to its requests. 

### ATP library:

The ATP library `libatp.so` provides an API to annotate applications and extract control variables details during run-time.

In order to instrument application with ATP atplib.h file should be
used. atplib.h provides fallowing macros to instument the applciation
      
      ATP_PARAM_DECLARE(_pname, _ptype, _dlt_val, _domain) - create a new
      parameter with name _pname of type _ptype with default value _dlt_val un
      domain _domain. Supported typs are "range" and "enum".

      ATP_PARAM_ADD_VALUES(_pname, _vArray, _nValues, _domain) - add values to
      parameter _pname in _domain. _vArray is the array of values to add to
      parameter and _nValues is the size of this array. If type is range
      _vArray should be 3 element array containig 3 ints {min, max, step}. If
      the type is enum _vArray is a list of possible numbers and _nValues is
      hte size of this list. 

      ATP_CONSTRAINT_DECLARE(_cname, _expr, _domain) - declare a constraint
      that will be aplied to getvalidparams,_domain; query for this domain.

      ATP_PARAM_GET(_pname, _address, _domain) - get the value of the
      parameter _pname in _domain and store in varuable _address


### ATP server:

ATP server provides responses to two queries:

1) getatpspecs;

This apt query responds with 5 messages:

     * first message contains an array of 3 ints

       - total number of ATP domains

       - total number of ATP parameters

       - total number of ATP parameter values

     * second message is and int array of size number of ATP domains, and
     contains the number of parameter in each domain

     * the third message is a char array of domain names, the size of the
     message is number of domains * 100, the name of each domain can be
     accesed by offseting the array by (number of domain * 100)

     * the forth message is a char array of parameter names, the size of the
     message is number of parameters * 100, the name of each parameter can be
     accesed by offseting the araay by (number of parameter * 100)

     * the fifth message is the parameter details:
       for each parameter the array contains - parameter type, default value,
       number of values and 'values'.

       example: if parameter type is "range" the information for each
       parameter  contains: type (0), default, num values (3), min, max, step. 


2) getvalidparams,domain-name;

   This query returns all the available combinatins that parameters can have
   in a single domain. The response to this query is two messages:
   
       * first message contains 3 int values
         
         - success code 1
         
	     - number of valid combinations
	 
         - elements per combination (equal to number of parameters in domain)
       
       * the second message is an int array of size (number of valid 
         comibnations)*(number of valid combinations), where each entry is 
         the value of the parameter in one of the combinations  

## Authors

Zakaria Bendifallah (zakaria.bendifallah at intel.com)

Uldis Locans (uldis.locans at intel.com)
