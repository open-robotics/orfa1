#ifndef ERRORS_H
#define ERRORS_H

//! error codes
typedef enum {
    NO_ERROR=0,         //!< no error
    INTERNAL_ERROR,     //!< internal parser error
    INVALID_COMMAND,    //!< unknown command
    INVALID_XDIGIT,     //!< non hex character
    INVALID_DATA,       //!< invalid data
    P_EXPECTED,         //!< 'P' expected (i2c)
} error_code_t;

#endif // ERRORS_H
