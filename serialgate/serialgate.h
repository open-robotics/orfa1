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

#ifndef SERAILGATE_H
#define SERAILGATE_H

void serialgate_mainloop(void);

#endif // SERAILGATE_H

