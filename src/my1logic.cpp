//------------------------------------------------------------------------------
#include "my1logic.hpp"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Logic::my1Logic(my1logic_t aLogic)
{
	mLogic = aLogic;
}
//------------------------------------------------------------------------------
my1Logic::my1Logic(my1Logic& aLogic)
{
	this->mLogic = aLogic.mLogic;
}
//------------------------------------------------------------------------------
my1Logic my1Logic::resolve(void)
{
	my1Logic cLogic;
	switch(this->mLogic)
	{
		case strong_1:
		case weak_1:
		case logic_1:
			cLogic = logic_1;
			break;
		case strong_0:
		case weak_0:
		case logic_0:
			cLogic = logic_0;
			break;
		default:
			cLogic = unknown;
	}
	return cLogic;
}
//------------------------------------------------------------------------------
bool my1Logic::operator==(my1logic_t aLogic)
{
	return this->mLogic==aLogic;
}
//------------------------------------------------------------------------------
bool my1Logic::operator!=(my1logic_t aLogic)
{
	return this->mLogic!=aLogic;
}
//------------------------------------------------------------------------------
my1Logic my1Logic::operator=(my1logic_t aLogic)
{
	mLogic = aLogic;
}
//------------------------------------------------------------------------------
my1Logic my1Logic::operator=(my1Logic& aLogic)
{
	this->mLogic = aLogic.mLogic;
}
//------------------------------------------------------------------------------
my1Logic my1Logic::operator!(void)
{
	my1Logic cLogic = this->resolve();
	switch(cLogic.mLogic)
	{
		case logic_1:
			cLogic = logic_0;
			break;
		case logic_0:
			cLogic = logic_1;
			break;
		default:
			cLogic = unknown;
			break;
	}
	return cLogic;
}
//------------------------------------------------------------------------------
my1Logic my1Logic::operator|(my1Logic& aLogic)
{
	my1Logic cLogicA = this->resolve();
	my1Logic cLogicB = aLogic.resolve();
	if(cLogicA==unknown||cLogicB==unknown)
		cLogicA = unknown;
	else if(cLogicA==logic_1||cLogicB==logic_1)
		cLogicA = logic_1;
	else
		cLogicA = logic_0;
	return cLogic;
}
//------------------------------------------------------------------------------
my1Logic my1Logic::operator&(my1Logic& aLogic)
{
	my1Logic cLogicA = this->resolve();
	my1Logic cLogicB = aLogic.resolve();
	if(cLogicA==unknown||cLogicB==unknown)
		cLogicA = unknown;
	else if(cLogicA==logic_0||cLogicB==logic_0)
		cLogicA = logic_0;
	else
		cLogicA = logic_1;
	return cLogic;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
