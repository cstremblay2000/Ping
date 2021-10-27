/**
 * File:        set_uid.h
 * Author:      Chris Tremblay <cst1465@rit.edu>
 * Date:        10/06/2021, National Noodle Day! 
 * Description:
 *      A library of functions to play with setuid and 
 *      implement some basic privilege management
 */

#ifndef SETUID_H_ 
#define SETUID_H_

#include <stdlib.h>

// Description:
//      Attempts to lower privileges to the one one supplied
// Parameters:
//      new_uid -> the new_uid you would like to be
// Returns:
//      0 if it was possible, < 0 otherwise
int drop_priv_temp(uid_t new_uid);

// Description:
//     Restore privileges back to root level
// Returns:
//     0 if possible, < 0 otherwise
int restore_priv( void );

// Description:
//      Prints the privileges in a pretty manner
void print_privileges( void );

#endif
