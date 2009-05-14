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

/**
 * @mainpage Описание
 * 
 *
 * Цель создания этих интерфейсов - изоляция кода, определяющего порты и 
 * устройства ввода/вывода, доступные в шлюз-контроллере, от кода, 
 * выполняющего разбор команд. Это делает структуру ПО шлюза более модульной
 * и упрощает перенос ПО на новые устройства, с другим набором доступных портов
 * и устройств ввода-вывода.
 *
 * В настоящее время интерфейсы привязаны к устройствам на основе
 * микроконтроллеров (далее - МК) Atmel AVR. Возможно создание уровня
 * абстракции, позволяющего использовать и другие МК.
 *
 * @par Порты и устройства ввода/вывода
 * 
 * Аппаратное обеспечение шлюза представляется в виде набора портов и устройств
 * ввода-вывода.
 *
 * Порт представляет собой группу выводов МК, которыми можно управлять совместно
 * или каждым по отдельности. Группировка выводов в порты соответствует 
 * структуре примененного МК. Интерфейсы доступа к портам позволяют 
 * использовать выводы портов для цифрового ввода/вывода.
 *
 *
 * Устройство ввода/вывода позволяет управлять группой выводов МК по
 * определенному алгоритму. Устройство ввода/вывода может быть как привязано
 * к внутреннему устройству МК (например, SPI, АЦП или таймер с ШИМ), так и
 * реализовывать собственный алгоритм работы (например, управление 
 * электродвигателями). Каждому устройству ставится в соответствие набор
 * \e регистров. Регистр - это уникальный идентификатор. Драйвер устройства
 * предоставляет набор функций, позволяющих осуществлять запись в регистр и
 * чтение из регистра. Функции, выполняемые устройством при записи или чтении
 * того или иного регистра, целиком определяются драйвером устройства.
 *
 */

