
/** ORC32 parsers
 * Parsers list:
 *   - '#' -- set position
 */

#include "eterm.h"
#include "hal/servo.h"

static bool servo_move_parser(char c, bool reinit) {
//	uint8_t servo;
//	uint16_t position;
//	uint16_t speed;
//	uint16_t time;

	return c == '\n';
}

static bool query_status_parser(char c, bool reinit) {

#define QSP_SELECT_CMD 100
#define QSP_Q_ERROR 101
#define QSP_QP_SN_ERROR 102

	static uint8_t servo_num;

	if(reinit){
		servo_num=QSP_SELECT_CMD;
		return false;
	};

	if( servo_num==QSP_SELECT_CMD )
	{
		if( c=='\n' )
		{
			if( servo_lld_is_done() )
			{
				printf(".");
			}else{
				printf("+");
			};
			return true;
		};
		if( c=='P' )
		{
			servo_num=0;
			return false;
		}
		if( c!=' ' ) servo_num=QSP_Q_ERROR;
		return false;
	};

	if( servo_num==QSP_Q_ERROR )
	{
		if( c!='\n' ) return false;
		printf("ERR in Q cmd\n");
		return true;
	};

	if( servo_num==QSP_QP_SN_ERROR )
	{
		if( c!='\n' ) return false;
		printf("ERR in QP cmd servo #\n");
		return true;
	};

	if( servo_num<100 ){
		if( c>='0' && c<='9' )
		{
			if(servo_num>=10)
			{
				servo_num=QSP_QP_SN_ERROR;
				return false;
			};
			servo_num=servo_num*10+(c-'0');
			return false;
		};
		if( c=='\n' )
		{
			char r=servo_get_position(servo_num)/10;
			putchar(r);
			return true;
		};
		if( c!=' ' ) servo_num=QSP_QP_SN_ERROR;
		return false;
	};

	return c == '\n';
}

static parser_t orc32parsers[] = {
	PARSER_INIT('#', "SSC-32 servo move", servo_move_parser),
	PARSER_INIT('Q', "SSC-32 query global status", query_status_parser)
};

void register_orc32(void) {
	register_parser(&orc32parsers[0]);
}

