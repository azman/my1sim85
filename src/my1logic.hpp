//------------------------------------------------------------------------------
// Class for Logic Signals and Devices
//------------------------------------------------------------------------------
#ifndef __MY1LOGICHPP
#define __MY1LOGICHPP
//------------------------------------------------------------------------------
// based on VHDL std_logic
enum my1logic_t { uninitialized='U', unknown='X', logic_1='1', logic_0='0',
	high_z='Z', strong_1='H', strong_0='L', weak_1='h', weak_0='l',
	weak_x='W', dontcare='-' };
//------------------------------------------------------------------------------
class my1Logic // basic logic data
{
	protected:
		my1logic_t mLogic;
	public:
		my1Logic(my1logic_t aLogic=uninitialized);
		my1Logic(my1Logic&);
		virtual ~my1Logic(){}
		my1Logic resolve(void);
		bool operator==(my1logic_t);
		bool operator!=(my1logic_t);
		my1Logic operator=(my1logic_t);
		my1Logic operator=(my1Logic&);
		my1Logic operator!(void);
		my1Logic operator|(my1Logic&);
		my1Logic operator&(my1Logic&);
};
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
