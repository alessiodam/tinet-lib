/*
 *--------------------------------------
 * Program Name: TINET lib
 * Author: TKB Studios
 * License: Mozilla Public License Version 2.0
 * Description: Allows the user to communicate with the TINET servers
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
 * Total hours wasted here = 0 hours.
 */

#ifndef TINET_H
#define TINET_H

#include <srldrvce.h>

#ifdef __cplusplus
extern "C"
{
#endif
  /**
   * @brief Possible return codes for TINET functions.
   */
  typedef enum
  {
    TINET_SUCCESS,                 /**< General return code for a successful operation. */
    TINET_FAIL,                    /**< General error code for unknown failures. */
    TINET_NO_KEYFILE,              /**< No keyfile found. */
    TINET_INVALID_KEYFILE,         /**< Invalid keyfile format. */
    TINET_TIMEOUT_EXCEEDED,        /**< Timeout exceeded during execution. */
    TINET_LOGIN_FAILED,            /**< General error code for a failed login. */
    TINET_NO_CONNECTION,           /**< General error code for no internet connection */
  } TINET_ReturnCode;

  /**
   * @brief Initializes the TINET system.
   * @return TINET_ReturnCode indicating the initialization result.
   */
  int tinet_init();

  /**
   * @brief Logs in to the TINET account.
   * @param timeout Timeout (in seconds) for login.
   * @attention this should only be run after a successful init and connect!
   */
  TINET_ReturnCode tinet_login(const int timeout);

  /* GETTERS */
  /**
   * @brief Gets the username associated with the TINET system.
   * @return A string representing the username.
   */
  char *tinet_get_username();

  /**
   * @brief Get the session token generated for this session.
   * @attention this should only be run after a successful init and connect!
   */
  char tinet_get_session_token();
#ifdef __cplusplus
}
#endif

#endif
