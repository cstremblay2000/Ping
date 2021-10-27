/**
 * File:        set_uid.c
 * Author:      Chris Tremblay <cst1465@rit.edu>
 * Date:        10/06/2021, National Noodle Day!
 * Description:
 *      A library of functions to play with setuid and 
 *      implement some basic privilege management
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "set_uid.h"

// template string for print privileges
const char* PRINT_PRIV = "\truid %d euid %d suid %d\n";

// The priv uid
uid_t PRIV_UID;


// Description:
//      Attempts to lower privileges to the one one supplied
// Parameters:
//      new_uid -> the new_uid you would like to be
// Returns:
//      0 if it was possible, < 0 otherwise
int drop_priv_temp(uid_t new_uid) {
        int old_euid = geteuid();
        // copy euid to suid
        if (setreuid(getuid(), old_euid) < 0)
                return -1;
        // set euid as new_uid
        if (seteuid(new_uid) < 0)
                return -1;
        if (geteuid() != new_uid)
                return -1;
        PRIV_UID = old_euid;
        return 0;
}

// Description:
//     Restore privileges back to root level
// Returns:
//     0 if possible, < 0 otherwise
int restore_priv( void ) {
        if (seteuid(PRIV_UID) < 0)
                return -1;
        if (geteuid() != PRIV_UID)
                return -1;
        return 0;
}


// Description:
//      Prints the privileges in a pretty manner
void print_privileges( void ){
        uid_t euid, ruid, suid;
        getresuid(&ruid, &euid, &suid);
        printf( PRINT_PRIV, ruid, euid, suid );
        return;
}

