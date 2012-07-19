Open Robotics Firmware Architecture
===================================

Firmware for Open Robotics controller boards.


Documentation
-------------

See documentation at:
* RU:
  * WIKI: http://robofourm.ru/wiki/ORFA
  * User Manual: http://hg.vehq.ru/orfa-manual (LaTeX source)


Requirements
------------

* avr-gcc > 4.2
* avr-libc > 1.6
* binutils-avr
* avrdude (optional)


Installation
------------

First configure youre local_config.mk:

 $ make config
 $ edit local_config.mk

Compile:

 $ make

You also can program this firmware into flash
(see PROGRAMMER section of local_config.mk):

 $ make program

