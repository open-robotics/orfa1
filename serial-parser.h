#ifndef SERIAL_PARSER_H
#define SERIAL_PARSER_H

#include <stdint.h>
#include <stdbool.h>

#include "errors.h"
#include "cbuf.h"

typedef enum {
    CMD_INIT,       //!< clear cmd buffer
    GET_COMMAND,    //!< get command ( 'S', 'R', 'W', '#', '\r', '\n' )
    COMMENT_EOL,    //!< skip all chars, wait '\r' or '\n', change state to CMD_INIT
    PARSE_CONFIG,   //!< parse config command ( C<f1><f2> )
    PARSE_LOCAL,    //!< parse set local addr command  ( L<aa> )
    PARSE_I2C,      //!< parse I2C commands ( S<aa><data>[S<aa><data>]P )
    PARSE_READ,     //!< parse read register command ( R<aa><rr>[<ll>] )
    PARSE_WRITE,    //!< parse write register command ( W<aa><rr><data> )
    WAIT_EOL,       //!< skip all chars, wait '\r' or '\n', change state to EXEC_COMMAND
    EXEC_COMMAND,   //!< start exec, change state to CMD_INIT
} state_cmd_t;

typedef enum {
    GET_ADDRESS,            //!< get address (both register and i2c machine)
    GET_REGISTER,           //!< get register (register machine)
    GET_LENGTH_OR_EOF,      //!< get length (register machine)
    GET_DATA_OR_EOF,        //!< get data (register machine)
    PARSE_I2C_READ,         //!< parse i2c read, change state to WAIT_RESTART_OR_STOP
    PARSE_I2C_WRITE,        //!< parse i2c write
    WAIT_RESTART_OR_STOP,   //!< wait next i2c frame or stop
} state_i2creg_t;


bool parse_cmd(uint8_t co, cbf_t *cmd_buf, error_code_t *error_code);


#endif // SERIAL_PARSER_H
