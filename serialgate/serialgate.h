/*
 *  ORFA -- Open Robotics Firmware Architecture
 *
 *  Copyright (c) 2009 Vladimir Ermakov, Andrey Demenev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *****************************************************************************/

/** Serial-to-I2C gate
 *
 * @p Read i2c regex
 * <aa> — i2c address & 0xFE
 * <ll> — read count
 * @code
 * S[0-9a-fA-F]{2}<aa>[0-9a-fA-F]{2}<ll>
 * @endcode
 *
 * @p Write i2c regex
 * <aa> — i2c address | 0x01
 * <dd> — write data
 * @code
 * S[0-9a-fA-F]{2}<aa>((\\.|[0-9a-fA-F]{2})<dd>)+
 * @endcode
 *
 * @p I2C regex
 * <read-i2c-regex> — see read i2c regex
 * <write-i2c-regex> — see write i2c regex
 * @code
 * (<read-i2c-regex>|<write-i2c-regex>)+P
 * @endcode
 *
 * @p Read register regex
 * <aa> — device address
 * <rr> — register
 * <ll> — read count
 * @code
 * R[0-9a-fA-F]{2}<aa>[0-9a-fA-F]{2}<rr>([0-9a-fA-F]{2}<ll>){0,1}
 * @endcode
 *
 * @p Write register regex
 * <aa> — device address
 * <rr> — register
 * <dd> — write data
 * @code
 * W[0-9a-fA-F]{2}<aa>[0-9a-fA-F]{2}<rr>((\\.|[0-9a-fA-F]{2})<dd>)+
 * @endcode
 */
// vim:set ts=4 sw=4 et:

#ifndef SERAILGATE_H
#define SERAILGATE_H

void serialgate_mainloop(void);

#endif // SERAILGATE_H

