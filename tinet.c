/*
*--------------------------------------
 * Program Name: TINET lib
 * Author: TKB Studios
 * License: Mozilla Public License Version 2.0
 * Description: Allows the user to communicate with the TINET servers
 *--------------------------------------
*/

/*
 *--------------Contributors--------------
 * TIny_Hacker - key parsing for login
 * ACagliano (Anthony Cagliano) - help with general serial things
 * commandblockguy - help with serial read
 *--------------------------------------
*/

/*
 * Dear programmer,
 * when I wrote this code, only God and I
 * knew how it worked.
 * Now, only God knows it!
 *
 * Therefore, if you are trying to optimize
 * this code and it fails (most surely),
 * please increase this counter as
 * a warning for the next person:
 *
 * Total hours wasted here = 17 hours.
*/

#include "tinet-lib/tinet.h"
#include <string.h>
#include <fileioc.h>
#include <time.h>

uint8_t NetKeyAppVar;
char *username;
char *calckey;
char *sessiontoken;

int tinet_init() {
    NetKeyAppVar = ti_Open("NETKEY", "r");
    if (NetKeyAppVar == 0) {
        return TINET_NO_KEYFILE;
    }

    uint8_t *data_ptr = ti_GetDataPtr(NetKeyAppVar);
    ti_Close(NetKeyAppVar);

    username = (char *)data_ptr;
    calckey = (char *)(data_ptr + strlen(username) + 1);

    return TINET_SUCCESS;
}

char* tinet_get_username() {
    return username;
}

TINET_ReturnCode tinet_login(const int timeout) {
    time_t start_time;
    time_t current_time;
    time(&start_time);

    do {
        time(&current_time);
        if ((int)difftime(current_time, start_time) > timeout) {
            return TINET_TIMEOUT_EXCEEDED;
        }
        // request a new session token
        // set sessiontoken to variable
    } while (1);

    // key shouldn't be used again after a successful session token request
    calckey[0] = '\0';
    return TINET_SUCCESS;
}

char tinet_get_session_token() {
    return *sessiontoken;
}
