/*! Serial-to-I2C command parser
 *
 * \p Read i2c regex
 * <aa> — i2c address & 0xFE
 * <ll> — read count
 * \code
 * S[0-9a-fA-F]{2}<aa>[0-9a-fA-F]{2}<ll>
 * \endcode
 *
 * \p Write i2c regex
 * <aa> — i2c address | 0x01
 * <dd> — write data
 * \code
 * S[0-9a-fA-F]{2}<aa>((\\.|[0-9a-fA-F]{2})<dd>)+
 * \endcode
 *
 * \p I2C regex
 * <read-i2c-regex> — see read i2c regex
 * <write-i2c-regex> — see write i2c regex
 * \code
 * (<read-i2c-regex>|<write-i2c-regex>)+P
 * \endcode
 *
 * \p Read register regex
 * <aa> — device address
 * <rr> — register
 * <ll> — read count
 * \code
 * R[0-9a-fA-F]{2}<aa>[0-9a-fA-F]{2}<rr>([0-9a-fA-F]{2}<ll>){0,1}
 * \endcode
 *
 * \p Write register regex
 * <aa> — device address
 * <rr> — register
 * <dd> — write data
 * \code
 * W[0-9a-fA-F]{2}<aa>[0-9a-fA-F]{2}<rr>((\\.|[0-9a-fA-F]{2})<dd>)+
 * \endcode
 *
 * vim:set ts=4 sw=4 et:
 */

#include <stdint.h>
#include <stdio.h>

#include "errors.h"
#include "cbuf.h"
#include "serial-parser.h"
#include "serial-command.h"

int main()
{
    uint8_t c;

    //! command buffer
    cbf_t cmd_buf, tx_buf;

    //! error state
    error_code_t error_code=NO_ERROR;

    cbf_init(&cmd_buf);
    cbf_init(&tx_buf);

    for(;;)
    {
        c = getchar();

        if(parse_cmd(c, &cmd_buf, &error_code))
        {
            exec_cmd(&cmd_buf, &tx_buf);
            while(!cbf_isempty(&tx_buf))
            {
                putchar(cbf_get(&tx_buf));
            }
        }

        if(error_code != NO_ERROR)
        {
            printf("ERROR: %i\n", error_code);

            error_code = NO_ERROR;
        }
    } // for

} // main()
