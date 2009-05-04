/*
 *  userial -- a USB to I2C converter
 *
 *  Copyright (c) 2008-2009, Thomas Pircher <tehpeh@gmx.net>
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
#include "cbuf.h"
#include <stdint.h>
#include <stdbool.h>

void cbf_init(cbf_t *cbf) {
    cbf->pread = cbf->pwrite = cbf->data;
}


void cbf_put(cbf_t *cbf, uint8_t data)
{
    if (!cbf_isfull(cbf)) {
        *cbf->pwrite++ = data;
        if (cbf->pwrite >= cbf->data + CBF_SIZE) {
            cbf->pwrite = cbf->data;
        }
    }
}


uint8_t cbf_get(cbf_t *cbf)
{
    uint8_t ret = 0;
    if (!cbf_isempty(cbf)) {
        ret = *cbf->pread++;
        if (cbf->pread >= cbf->data + CBF_SIZE) {
            cbf->pread = cbf->data;
        }
    }
    return ret;
}

uint8_t cbf_peek(const cbf_t *cbf)
{
    return cbf_isempty(cbf) ? 0 : *cbf->pread;
}


int cbf_find(const cbf_t *cbf, uint8_t c)
{
    int count = 0;
    const uint8_t *pread = cbf->pread;

    while (true) {
        if (pread == cbf->pwrite) {
            return -1;
        }

        if (*pread == c) {
            return count;
        }
        count++;

        pread++;
        if (pread >= cbf->data + CBF_SIZE) {
            pread = cbf->data;
        }
    }
}
