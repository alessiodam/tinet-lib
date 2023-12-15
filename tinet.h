/*
 *--------------------------------------
 * Program Name: TINET lib
 * Author: TKB Studios
 * License: Mozilla Public License Version 2.0
 * Description: Allows the user to communicate with the TINET servers
 *--------------------------------------
*/

#ifndef TINET_H
#define TINET_H

#include <srldrvce.h>

#ifdef __cplusplus
extern "C" {
#endif

 /**
  * @brief Indicates whether a serial device is present.
  */
 extern bool has_srl_device;

 /**
  * @brief Indicates whether a connection to TINET bridge is established.
  */
 extern bool bridge_connected;

 /**
  * @brief Possible return codes for TINET functions.
  */
 typedef enum {
  TINET_SUCCESS,            /**< Operation succeeded. */
  TINET_NO_KEYFILE,         /**< No keyfile found. */
  TINET_INVALID_KEYFILE,    /**< Invalid keyfile format. */
  TINET_SRL_INIT_FAIL,      /**< Serial initialization failure. */
  TINET_SRL_WRITE_FAIL,     /**< Serial write failure. */
  TINET_SRL_READ_FAIL,      /**< Serial read failure. */
  TINET_NO_BRIDGE,          /**< No bridge connected or not initialized. */
  TINET_TIMEOUT_EXCEEDED,   /**< Timeout exceeded during execution. */
 } TINET_ReturnCode;

 /**
  * @brief Initializes the TINET system.
  * @return TINET_ReturnCode indicating the initialization result.
  */
 int tinet_init();

 /**
  * @brief Gets the username associated with the TINET system.
  * @return A string representing the username.
  */
 char* tinet_get_username();

 /**
  * @brief Establishes a connection to the TINET server.
  * @return TINET_ReturnCode indicating the connection result.
  */
 int tinet_connect(int timeout);

 /**
  * @brief Retrieves the serial device used by TINET.
  * @return The serial device structure.
  */
 srl_device_t tinet_get_srl_device();

 /**
  * @brief Writes a message to the serial device.
  * @param message The message to be written.
  * @return TINET_ReturnCode indicating the write result.
  */
 int tinet_write_srl(const char *message);

 /**
  * @brief Reads data from the serial device.
  * @param to_buffer The buffer to store the read data.
  * @return TINET_ReturnCode indicating the read result or the amount of bytes written.
  */
 int tinet_read_srl(char *to_buffer);

#ifdef __cplusplus
}
#endif

#endif
