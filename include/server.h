#pragma once

#include "csound.h"

/**
 * Starts the UDP server on a supplied port number
 * returns CSOUND_SUCCESS if server has been started successfully,
 * otherwise, CSOUND_ERROR.
 */
PUBLIC int csoundUDPServerStart(CSOUND *csound, unsigned int port);

/** returns the port number on which the server is running, or
 *  CSOUND_ERROR if the server is not running.
 */
PUBLIC int csoundUDPServerStatus(CSOUND *csound);

/**
 * Closes the UDP server, returning CSOUND_SUCCESS if the
 * running server was successfully closed, CSOUND_ERROR otherwise.
 */
PUBLIC int csoundUDPServerClose(CSOUND *csound);

/**
 * Turns on the transmission of console messages to UDP on address addr
 * port port. If mirror is one, the messages will continue to be
 * sent to the usual destination (see csoundSetMessaggeCallback())
 * as well as to UDP.
 * returns CSOUND_SUCCESS or CSOUND_ERROR if the UDP transmission
 * could not be set up.
 */
PUBLIC int csoundUDPConsole(CSOUND *csound, const char *addr, int port,
                            int mirror);

/**
 * Stop transmitting console messages via UDP
 */
PUBLIC void csoundStopUDPConsole(CSOUND *csound);