//------------------------------------------------------------------------------
#include "my1sim85.hpp"
//------------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
//#include <ctime>
#include <fstream>
//------------------------------------------------------------------------------
#define PROGNAME "my1sim85"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1SimObject::my1SimObject()
{
	mID = -1;
	mName[0] = 0x0;
	mLink = 0x0;
	DoUpdate = 0x0;
	DoDetect = 0x0;
	DoDelay = 0x0;
}
//------------------------------------------------------------------------------
int my1SimObject::GetID(void)
{
	return mID;
}
//------------------------------------------------------------------------------
void my1SimObject::SetID(int anID)
{
	mID = anID;
}
//------------------------------------------------------------------------------
const char* my1SimObject::GetName(void)
{
	return (const char*) mName;
}
//------------------------------------------------------------------------------
void my1SimObject::SetName(const char* aName)
{
	int cCount = 0;
	if(aName)
	{
		while(aName[cCount]&&cCount<MAX_SIMNAME_SIZE-1)
		{
			mName[cCount] = aName[cCount];
			cCount++;
		}
	}
	mName[cCount] = 0x0;
}
//------------------------------------------------------------------------------
void* my1SimObject::GetLink(void)
{
	return mLink;
}
//------------------------------------------------------------------------------
void my1SimObject::SetLink(void* aLink)
{
	mLink = aLink;
}
//------------------------------------------------------------------------------
void my1SimObject::Unlink(void)
{
	mLink = 0x0;
	DoUpdate = 0x0;
	DoDetect = 0x0;
}
//------------------------------------------------------------------------------
abyte my1SimObject::RandomByte(void)
{
	return rand() % 0x100;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Address::my1Address(int aStart, int aSize)
{
	if(aStart<0) mStart = 0;
	else mStart = aStart;
	if(aSize>0&&aSize<=MAX_MEMSIZE) mSize = aSize;
	else mSize = 0;
	mNext = 0x0;
}
//------------------------------------------------------------------------------
int my1Address::GetStart(void)
{
	return mStart;
}
//------------------------------------------------------------------------------
int my1Address::GetSize(void)
{
	return mSize;
}
//------------------------------------------------------------------------------
my1Address* my1Address::Next(void)
{
	return mNext;
}
//------------------------------------------------------------------------------
void my1Address::Next(my1Address* aNext)
{
	mNext = aNext;
}
//------------------------------------------------------------------------------
bool my1Address::IsOverlapped(int aStart, int aSize)
{
	bool cFlag = false;
	if((aStart>=mStart&&aStart<mStart+mSize)||
		(mStart>=aStart&&mStart<aStart+aSize))
		cFlag = true;
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Address::IsOverlapped(my1Address& rAddress)
{
	return this->IsOverlapped(rAddress.mStart,rAddress.mSize);
}
//------------------------------------------------------------------------------
bool my1Address::IsSelected(aword anAddress)
{
	if(anAddress>=mStart&&anAddress<mStart+mSize)
		return true;
	return false;
}
//------------------------------------------------------------------------------
void my1Address::Reset(bool aCold)
{
	return; // by default, do nothing!
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Memory::my1Memory(int aStart, int aSize, bool aRandomize, bool aROM)
	: my1Address(aStart,aSize)
{
	mReadOnly = aROM;
	mProgramMode = false;
	mLastUsed = 0x0; // let random?
	if(mSize>0) mSpace = new abyte[mSize];
	else mSpace = 0x0;
	if(aRandomize) this->Randomize();
}
//------------------------------------------------------------------------------
my1Memory::~my1Memory()
{
	if(mSpace) delete[] mSpace;
}
//------------------------------------------------------------------------------
void my1Memory::Randomize(void)
{
	for(int cLoop=0;cLoop<mSize;cLoop++)
		mSpace[cLoop] = this->RandomByte();
}
//------------------------------------------------------------------------------
bool my1Memory::IsReadOnly(void)
{
	return mReadOnly;
}
//------------------------------------------------------------------------------
void my1Memory::ProgramMode(bool aStatus)
{
	mProgramMode = aStatus;
}
//------------------------------------------------------------------------------
int my1Memory::GetLastUsed(void)
{
	return mLastUsed+mStart; // give address!
}
//------------------------------------------------------------------------------
abyte my1Memory::GetLastData(void)
{
	return mSpace[mLastUsed];
}
//------------------------------------------------------------------------------
bool my1Memory::GetData(aword anAddress, abyte& rData)
{
	if(!this->IsSelected(anAddress)) return false;
	rData = mSpace[anAddress-mStart];
	return true;
}
//------------------------------------------------------------------------------
bool my1Memory::ReadData(aword anAddress, abyte& rData)
{
	if(!this->IsSelected(anAddress)) return false;
	mLastUsed = anAddress-mStart;
	rData = mSpace[mLastUsed];
	if(DoDelay) // example to implement access delay!
		(*DoDelay)((void*)this,1);
	return true;
}
//------------------------------------------------------------------------------
bool my1Memory::WriteData(aword anAddress, abyte aData)
{
	if(mReadOnly&&!mProgramMode) return false;
	if(!this->IsSelected(anAddress)) return false;
	mLastUsed = anAddress-mStart;
	mSpace[mLastUsed] = aData;
	if(DoUpdate) // update sim gui if required
		(*DoUpdate)((void*)this);
	return true;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Sim2764::my1Sim2764(int aStart)
	: my1Memory(aStart,I2764_SIZE,true)
{
	this->SetName(I2764_NAME);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Sim6264::my1Sim6264(int aStart)
	: my1Memory(aStart,I6264_SIZE)
{
	this->SetName(I6264_NAME);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1BitIO::my1BitIO(void)
{
	mInput = false;
	// mState = this->RandomByte() % 2 ? BIT_STATE_1 : BIT_STATE_0;
}
//------------------------------------------------------------------------------
bool my1BitIO::IsInput(void)
{
	return mInput;
}
//------------------------------------------------------------------------------
void my1BitIO::SetInput(bool anInput)
{
	mInput = anInput;
}
//------------------------------------------------------------------------------
abyte my1BitIO::GetState(void)
{
	return mState;
}
//------------------------------------------------------------------------------
void my1BitIO::SetState(abyte aState)
{
	mState = aState;
}
//------------------------------------------------------------------------------
abyte my1BitIO::GetData(void)
{
	if(!mInput)
		return mState; // buffered output
	if(DoDetect)
		(*DoDetect)((void*)this);
	return mState;
}
//------------------------------------------------------------------------------
void my1BitIO::SetData(abyte aData)
{
	if(mInput)
		return;
	mState = aData;
	if(DoUpdate)
		(*DoUpdate)((void*)this);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1BitIO* my1DevicePort::GetBitIO(int anIndex)
{
	return &mDevicePins[anIndex];
}
//------------------------------------------------------------------------------
abyte my1DevicePort::IsInput(void)
{
	abyte cFlag = 0x00, cCheck = 0x01;
	for(int cLoop=0;cLoop<MAX_PORTPIN_COUNT;cLoop++)
	{
		if(mDevicePins[cLoop].IsInput())
			cFlag |= cCheck;
		cCheck <<= 1;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
void my1DevicePort::SetInput(bool anInput, abyte aMask)
{
	abyte cCheck = 0x01;
	for(int cLoop=0;cLoop<MAX_PORTPIN_COUNT;cLoop++)
	{
		if(cCheck&aMask) continue; // skip if masked out!
		mDevicePins[cLoop].SetInput(anInput);
	}
}
//------------------------------------------------------------------------------
abyte my1DevicePort::GetPort(void)
{
	abyte cData = 0x0, cCheck = 0x01;
	for(int cLoop=0;cLoop<MAX_PORTPIN_COUNT;cLoop++)
	{
		if(mDevicePins[cLoop].GetState()==BIT_STATE_1)
			cData |= cCheck;
		cCheck <<= 1;
	}
	return cData;
}
//------------------------------------------------------------------------------
void my1DevicePort::SetPort(abyte aData)
{
	abyte cCheck = 0x01;
	for(int cLoop=0;cLoop<MAX_PORTPIN_COUNT;cLoop++)
	{
		if(cCheck&aData)
			mDevicePins[cLoop].SetState(BIT_STATE_1);
		else
			mDevicePins[cLoop].SetState(BIT_STATE_0);
		cCheck <<= 1;
	}
}
//------------------------------------------------------------------------------
abyte my1DevicePort::GetData(void)
{
	abyte cData = 0x0, cCheck = 0x01;
	if(DoDetect) // either here or in bitio!
		(*DoDetect)((void*)this);
	for(int cLoop=0;cLoop<MAX_PORTPIN_COUNT;cLoop++)
	{
		if(mDevicePins[cLoop].GetData()==BIT_STATE_1)
			cData |= cCheck;
		cCheck <<= 1;
	}
	return cData;
}
//------------------------------------------------------------------------------
void my1DevicePort::SetData(abyte aData)
{
	abyte cCheck = 0x01;
	for(int cLoop=0;cLoop<MAX_PORTPIN_COUNT;cLoop++)
	{
		if(cCheck&aData)
			mDevicePins[cLoop].SetData(BIT_STATE_1);
		else
			mDevicePins[cLoop].SetData(BIT_STATE_0);
		cCheck <<= 1;
	}
	if(DoUpdate) // either here or in bitio!
		(*DoUpdate)((void*)this);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Device::my1Device(int aStart, int aSize)
	: my1Address(aStart,aSize)
{
	if(mSize>0) mDevicePorts = new my1DevicePort[mSize];
	else mDevicePorts = 0x0;
}
//------------------------------------------------------------------------------
my1Device::~my1Device()
{
	if(mDevicePorts) delete[] mDevicePorts;
}
//------------------------------------------------------------------------------
my1DevicePort* my1Device::GetDevicePort(int anIndex)
{
	return &mDevicePorts[anIndex];
}
//------------------------------------------------------------------------------
bool my1Device::ReadDevice(abyte anAddress, abyte& rData)
{
	if(!this->IsSelected(anAddress)) return false;
	rData = mDevicePorts[anAddress-mStart].GetData();
	return true;
}
//------------------------------------------------------------------------------
bool my1Device::WriteDevice(abyte anAddress, abyte aData)
{
	if(!this->IsSelected(anAddress)) return false;
	mDevicePorts[anAddress-mStart].SetData(aData);
	return true;
}
//------------------------------------------------------------------------------
bool my1Device::ReadData(aword anAddress, abyte& rData)
{
	return this->ReadDevice(anAddress, rData);
}
//------------------------------------------------------------------------------
bool my1Device::WriteData(aword anAddress, abyte aData)
{
	return this->WriteDevice(anAddress, aData);
}
//------------------------------------------------------------------------------
abyte my1Device::GetData(int anIndex)
{
	return mDevicePorts[anIndex].GetData();
}
//------------------------------------------------------------------------------
void my1Device::PutData(int anIndex, abyte aData)
{
	mDevicePorts[anIndex].SetData(aData);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Sim8255::my1Sim8255(int aStart)
	: my1Device(aStart, I8255_SIZE)
{
	// by default config is random?
	this->SetName(I8255_NAME);
}
//------------------------------------------------------------------------------
bool my1Sim8255::ReadDevice(abyte anAddress, abyte& rData)
{
	if(!this->IsSelected(anAddress))
		return false;
	bool cFlag = true;
	int cIndex = anAddress-mStart;
	switch(cIndex)
	{
		case I8255_PORTA:
		case I8255_PORTB:
		case I8255_PORTC:
			rData = mDevicePorts[cIndex].GetData();
			break;
		case I8255_CNTRL:
			rData = mDevicePorts[cIndex].GetPort(); // NOT IO!
			break;
		default:
			cFlag = false;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim8255::WriteDevice(abyte anAddress, abyte aData)
{
	if(!this->IsSelected(anAddress))
		return false;
	bool cFlag = true, cCheck;
	int cIndex = anAddress-mStart;
	switch(cIndex)
	{
		case I8255_PORTA:
		case I8255_PORTB:
		case I8255_PORTC:
			mDevicePorts[cIndex].SetData(aData);
			break;
		case I8255_CNTRL:
			if(aData&0x80)
			{
				// config data - save this!
				mDevicePorts[I8255_CNTRL].SetPort(aData);
				// always assume mode 0!
				if(aData&0x10) cCheck = true;
				else cCheck = false;
				mDevicePorts[I8255_PORTA].SetInput(cCheck);
				if(aData&0x02) cCheck = true;
				else cCheck = false;
				mDevicePorts[I8255_PORTB].SetInput(cCheck);
				if(aData&0x08) cCheck = true;
				else cCheck = false;
				mDevicePorts[I8255_PORTC].SetInput(cCheck,0x0F);
				if(aData&0x01) cCheck = true;
				else cCheck = false;
				mDevicePorts[I8255_PORTC].SetInput(cCheck,0xF0);
			}
			else
			{
				// bsr mode!
				abyte cCount = aData&0x0e;
				abyte cData = aData&0x01;
				my1DevicePort* aPort = &mDevicePorts[I8255_PORTC];
				my1BitIO* aPin = aPort->GetBitIO(cCount);
				aPin->SetData(cData);
			}
			break;
		default:
			cFlag = false;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Reg85::my1Reg85(bool aReg16)
{
	mData = 0x00;
	mReg16 = aReg16;
	pHI = 0x0; pLO = 0x0;
}
//------------------------------------------------------------------------------
void my1Reg85::Randomize(void)
{
	mData = (aword) this->RandomByte() << 8;
	mData = mData | this->RandomByte();
}
//------------------------------------------------------------------------------
void my1Reg85::UsePair(my1Reg85* aReg, my1Reg85* bReg)
{
	pHI = aReg; pLO = bReg;
	if(pLO&&pHI)
		mReg16 = true;
}
//------------------------------------------------------------------------------
bool my1Reg85::IsReg16(void)
{
	return mReg16;
}
//------------------------------------------------------------------------------
aword my1Reg85::GetData(void)
{
	aword cValue;
	if(pLO&&pHI)
	{
		cValue = pHI->GetData()<<8 | pLO->GetData();
	}
	else
	{
		if(mReg16) cValue = mData;
		else cValue = mData&0xFF;
	}
	return cValue;
}
//------------------------------------------------------------------------------
void my1Reg85::SetData(aword aData)
{
	if(pLO&&pHI)
	{
		pHI->SetData((aData&0xFF00)>>8);
		pLO->SetData(aData&0xFF);
	}
	else
	{
		if(mReg16) mData = aData;
		else  mData = aData&0xFF;
		if(DoUpdate)
			(*DoUpdate)((void*)this);
	}
}
//------------------------------------------------------------------------------
aword my1Reg85::MaskData(aword aMask)
{
	this->SetData(this->GetData()&aMask);
	return mData;
}
//------------------------------------------------------------------------------
aword my1Reg85::Increment(bool aPrior)
{
	aword cTest = mData;
	this->SetData(this->GetData()+1);
	if(aPrior) cTest = mData;
	return cTest;
}
//------------------------------------------------------------------------------
aword my1Reg85::Decrement(bool aPrior)
{
	aword cTest = mData;
	this->SetData(this->GetData()-1);
	if(aPrior) cTest = mData;
	return cTest;
}
//------------------------------------------------------------------------------
aword my1Reg85::Accumulate(aword aData)
{
	this->SetData(this->GetData()+aData);
	return mData;
}
//------------------------------------------------------------------------------
my1Reg85& my1Reg85::operator=(my1Reg85& aReg)
{
	this->SetData(aReg.GetData());
	return *this;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Reg85Pair::my1Reg85Pair(my1Reg85* aReg, my1Reg85* bReg)
	: my1Reg85(true)
{
	this->UsePair(aReg,bReg);
}
//------------------------------------------------------------------------------
my1Reg85Pair::my1Reg85Pair(my1Reg85Pair& aPair)
	: my1Reg85(true)
{
	*this = aPair;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1AddressMap::my1AddressMap()
{
	mFirst = 0x0;
	mCount = 0;
	mMapSize = 0; // should be set by child class
}
//------------------------------------------------------------------------------
int my1AddressMap::GetCount(void)
{
	return mCount;
}
//------------------------------------------------------------------------------
int my1AddressMap::GetMapSize(void)
{
	return mMapSize;
}
//------------------------------------------------------------------------------
bool my1AddressMap::Insert(my1Address* anObject)
{
	// check if outside total address space!
	int cBegin = anObject->GetStart();
	int cTotal = cBegin + anObject->GetSize();
	if(cBegin<0||cTotal>mMapSize) return false;
	// get insert location based on start address
	my1Address *pTemp = mFirst, *pPrev = 0x0;
	while(pTemp)
	{
		if(cBegin<pTemp->GetStart())
			break;
		pPrev = pTemp;
		pTemp = pTemp->Next();
	}
	// insert in between pPrev & pTemp - check overlapped!
	if(pPrev&&pPrev->IsOverlapped(*anObject))
		return false;
	if(pTemp&&pTemp->IsOverlapped(*anObject))
		return false;
	// now, insert!
	if(!pPrev) // first object!
		mFirst = anObject;
	else
		pPrev->Next(anObject);
	anObject->Next(pTemp);
	mCount++;
	return true;
}
//------------------------------------------------------------------------------
my1Address* my1AddressMap::Remove(int aStart)
{
	// get object location based on start address
	my1Address *pTemp = mFirst, *pPrev = 0x0;
	while(pTemp)
	{
		if(aStart<0||aStart==pTemp->GetStart())
			break;
		pPrev = pTemp;
		pTemp = pTemp->Next();
	}
	if(pTemp)
	{
		// okay... remove!
		if(!pPrev)
			mFirst = pTemp->Next();
		else
			pPrev->Next(pTemp->Next());
		mCount--;
	}
	return pTemp;
}
//------------------------------------------------------------------------------
my1Address* my1AddressMap::Object(aword anAddress, int* pIndex)
{
	// get object location based on start address
	int cIndex = -1;
	my1Address *pTemp = mFirst;
	while(pTemp)
	{
		cIndex++;
		if(pTemp->IsSelected(anAddress))
			break;
		pTemp = pTemp->Next();
	}
	if(pIndex) *pIndex = cIndex;
	return pTemp;
}
//------------------------------------------------------------------------------
my1Address* my1AddressMap::Object(int anIndex, int* pAddress)
{
	// get object location based on index?
	int cIndex = -1;
	my1Address *pTemp = mFirst;
	while(pTemp)
	{
		cIndex++;
		if(cIndex==anIndex)
			break;
		pTemp = pTemp->Next();
	}
	if(pAddress)
	{
		if(pTemp) *pAddress = pTemp->GetStart();
		else *pAddress = -1;
	}
	return pTemp;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1MemoryMap85::my1MemoryMap85()
{
	mMapSize = MAX_MEMSIZE;
}
//------------------------------------------------------------------------------
my1Memory* my1MemoryMap85::Memory(aword anAddress)
{
	return (my1Memory*) this->Object(anAddress);
}
//------------------------------------------------------------------------------
void my1MemoryMap85::ProgramMode(bool aStatus)
{
	// browse through objects
	my1Address *pTemp = mFirst;
	while(pTemp)
	{
		my1Memory* pMem = (my1Memory*) pTemp;
		pMem->ProgramMode(aStatus);
		pTemp = pTemp->Next();
	}
}
//------------------------------------------------------------------------------
bool my1MemoryMap85::Read(aword anAddress, abyte& rData)
{
	bool cFlag = false;
	my1Memory *cMemory = this->Memory(anAddress);
	if(cMemory)
		cFlag = cMemory->ReadData(anAddress, rData);
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1MemoryMap85::Write(aword anAddress, abyte aData)
{
	bool cFlag = false;
	my1Memory *cMemory = this->Memory(anAddress);
	if(cMemory)
		cFlag = cMemory->WriteData(anAddress, aData);
	return cFlag;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1DeviceMap85::my1DeviceMap85()
{
	mMapSize = MAX_DEVSIZE;
}
//------------------------------------------------------------------------------
my1Device* my1DeviceMap85::Device(aword anAddress)
{
	return (my1Device*) this->Object(anAddress);
}
//------------------------------------------------------------------------------
bool my1DeviceMap85::Read(aword anAddress, abyte& rData)
{
	bool cFlag = false;
	my1Device *cDevice = this->Device(anAddress);
	if(cDevice)
		cFlag = cDevice->ReadData(anAddress, rData);
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1DeviceMap85::Write(aword anAddress, abyte aData)
{
	bool cFlag = false;
	my1Device *cDevice = this->Device(anAddress);
	if(cDevice)
		cFlag = cDevice->WriteData(anAddress, aData);
	return cFlag;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Sim8085::my1Sim8085()
	: mRegPSW(&mRegMAIN[I8085_REG_A],&mRegMAIN[I8085_REG_F])
{
	// give id to regs
	mRegMAIN[I8085_REG_B].SetID(I8085_REG_B);
	mRegMAIN[I8085_REG_C].SetID(I8085_REG_C);
	mRegMAIN[I8085_REG_D].SetID(I8085_REG_D);
	mRegMAIN[I8085_REG_E].SetID(I8085_REG_E);
	mRegMAIN[I8085_REG_H].SetID(I8085_REG_H);
	mRegMAIN[I8085_REG_L].SetID(I8085_REG_L);
	mRegMAIN[I8085_REG_F].SetID(I8085_REG_F);
	mRegMAIN[I8085_REG_A].SetID(I8085_REG_A);
	// initialize register pairs
	mRegPAIR[I8085_RP_BC].UsePair(&mRegMAIN[I8085_REG_B],&mRegMAIN[I8085_REG_C]);
	mRegPAIR[I8085_RP_DE].UsePair(&mRegMAIN[I8085_REG_D],&mRegMAIN[I8085_REG_E]);
	mRegPAIR[I8085_RP_HL].UsePair(&mRegMAIN[I8085_REG_H],&mRegMAIN[I8085_REG_L]);
	// set input pins
	mPins[I8085_PIN_TRAP].SetInput();
	mPins[I8085_PIN_I7P5].SetInput();
	mPins[I8085_PIN_I6P5].SetInput();
	mPins[I8085_PIN_I5P5].SetInput();
	// reset device
	this->Reset(true);
}
//------------------------------------------------------------------------------
void my1Sim8085::Reset(bool aCold)
{
	// internal flags
	mErrorRW = false;
	mErrorISA = false;
	mHalted = false;
	mIEnabled = false;
	mExecINTR = false;
	// random flip-flop? NO!
	mFlagTRAP =false;
	mFlagI7P5 =false;
	// check if cold restart
	if(aCold)
	{
		for(int cLoop=0;cLoop<I8085_REG_COUNT;cLoop++)
			mRegMAIN[cLoop].Randomize();
		mRegMAIN[I8085_REG_F].MaskData(I8085_FLAG_BITS);
		mRegPAIR[I8085_RP_SP].Randomize();
	}
	// certain registers need specific reset value
	mRegINTR.SetData(I8085_IMSK_ALL);
	mRegPC.SetData(0x0000);
	// memory reset (pin reset out??)
	for(int cLoop=0;cLoop<mMemoryMap.GetCount();cLoop++)
		mMemoryMap.Object(cLoop)->Reset(aCold);
	for(int cLoop=0;cLoop<mDeviceMap.GetCount();cLoop++)
		mDeviceMap.Object(cLoop)->Reset(aCold);
}
//------------------------------------------------------------------------------
abyte my1Sim8085::GetParity(abyte aData)
{
	abyte cOddParity = 0x01;
	for(int cLoop=0;cLoop<8;cLoop++)
	{
		cOddParity ^= (aData&0x01);
		aData >>= 1;
	}
	return cOddParity;
}
//------------------------------------------------------------------------------
abyte my1Sim8085::GetSrcData(abyte src)
{
	abyte cData;
	switch(src)
	{
		case I8085_REG_M:
			mErrorRW |= !mMemoryMap.Read(mRegPAIR[I8085_RP_HL].GetData(),cData);
 			break;
		default:
			cData = mRegMAIN[src].GetData(); 
	}
	return cData;
}
//------------------------------------------------------------------------------
void my1Sim8085::PutDstData(abyte dst, abyte aData)
{
	switch(dst)
	{
		case I8085_REG_M:
			mErrorRW |= !mMemoryMap.Write(mRegPAIR[I8085_RP_HL].GetData(),aData);
 			break;
		default:
			mRegMAIN[dst].SetData(aData);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::DoStackPush(aword* pData)
{
	abyte *pDataL = (abyte*) pData;
	abyte *pDataH = (abyte*) (pDataL+1);
	my1Reg85* pRegSP = &mRegPAIR[I8085_RP_SP];
	mErrorRW |= !mMemoryMap.Write(pRegSP->Decrement(true),*pDataH);
	mErrorRW |= !mMemoryMap.Write(pRegSP->Decrement(true),*pDataL);
}
//------------------------------------------------------------------------------
void my1Sim8085::DoStackPop(aword* pData)
{
	abyte *pDataL = (abyte*) pData;
	abyte *pDataH = (abyte*) (pDataL+1);
	my1Reg85* pRegSP = &mRegPAIR[I8085_RP_SP];
	mErrorRW |= !mMemoryMap.Read(pRegSP->Increment(),*pDataL);
	mErrorRW |= !mMemoryMap.Read(pRegSP->Increment(),*pDataH);
}
//------------------------------------------------------------------------------
void my1Sim8085::UpdateFlag(abyte cflag, abyte result)
{
	if(result==0x00) cflag |= I8085_FLAG_Z; // zero
	if(GetParity(result)) cflag |= I8085_FLAG_P; // parity flag
	cflag |= (result&I8085_FLAG_S); // sign flag
	mRegMAIN[I8085_REG_F].SetData(cflag);
}
//------------------------------------------------------------------------------
bool my1Sim8085::CheckFlag(abyte sel)
{
	bool cStatus = false;
	abyte cTest = 0x00, cFlag = mRegMAIN[I8085_REG_F].GetData();
	switch(sel&0x06)
	{
		case 0: cTest = I8085_FLAG_Z; break; // jnz, jz
		case 2: cTest = I8085_FLAG_C; break; // jnc, jc
		case 4: cTest = I8085_FLAG_P; break; // jpo, jpe
		case 6: cTest = I8085_FLAG_S; break; // jp, jm
	}
	cStatus = (cFlag&cTest) ? false : true;
	if(sel&0x01) cStatus = !cStatus;
	return cStatus;
}
//------------------------------------------------------------------------------
bool my1Sim8085::CheckEdge(my1BitIO& pin)
{
	bool cFlag = false;
	abyte prev = pin.GetState();
	abyte next = pin.GetData();
	if(prev==BIT_STATE_0&&next==BIT_STATE_1)
		cFlag = true;
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim8085::CheckInterrupt(void)
{
	// check positive edge trigger for trap & 7.5
	// since sim only check after every instruction, makes no difference!
	if(this->CheckEdge(mPins[I8085_PIN_TRAP]))
		mFlagTRAP = true;
	if(this->CheckEdge(mPins[I8085_PIN_I7P5]))
		mFlagI7P5 = true;
	// now, do actual checking!
	mExecINTR = false;
	// check trap? NON-MASKABLE... edge AND level trigger!
	if(mFlagTRAP&&mPins[I8085_PIN_TRAP].GetData())
	{
		this->ExecCALL(I8085_ISR_TRP);
		mExecINTR = true;
		mFlagTRAP = false;
	}
	else if(mIEnabled)
	{
		bool cMaskI7P5 = false, cMaskI6P5 = false, cMaskI5P5 = false;
		// check interrupt mask
		if(mRegINTR.GetData()&I8085_IMSK_7P5) cMaskI7P5 = true;
		if(mRegINTR.GetData()&I8085_IMSK_6P5) cMaskI6P5 = true;
		if(mRegINTR.GetData()&I8085_IMSK_5P5) cMaskI5P5 = true;
		// check interrupt pin status
		if(!cMaskI7P5&&mFlagI7P5)
		{
			this->ExecCALL(I8085_ISR_7P5);
			mExecINTR = true;
			mFlagI7P5 = false; // reset flip-flop
		}
		else if(!cMaskI6P5&&mPins[I8085_PIN_I6P5].GetData())
		{
			this->ExecCALL(I8085_ISR_6P5);
			mExecINTR = true;
		}
		else if(!cMaskI5P5&&mPins[I8085_PIN_I5P5].GetData())
		{
			this->ExecCALL(I8085_ISR_5P5);
			mExecINTR = true;
		}
	}
	if(mExecINTR)
		mIEnabled = false;
	return mExecINTR;
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecMOV(abyte dst, abyte src)
{
	this->PutDstData(dst,this->GetSrcData(src));
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecMOVi(abyte dst, abyte aData)
{
	this->PutDstData(dst,aData);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecALU(abyte sel, abyte src)
{
	this->ExecALUi(sel,GetSrcData(src));
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecALUi(abyte sel, abyte aData)
{
	abyte cFlag, cTemp = mRegMAIN[I8085_REG_A].GetData();
	// do the operation!
	if((sel&0x04)&&(sel!=0x07)) // logical excluding cmp!
	{
		cFlag = 0x00; // by default ac reset, cy reset!
		switch(sel&0x03)
		{
			case 0x00:
				cTemp &= aData;
				cFlag = 0x10; // ac set, cy reset!
				break;
			case 0x01:
				cTemp ^= aData;
				break;
			case 0x02:
				cTemp |= aData;
				break;
		}
		mRegMAIN[I8085_REG_A].SetData(cTemp);
	}
	else // arithmetic
	{
		aword cTestX, cTestY; // for carry bits detection!
		if(sel==0x07) // actually a compare, do subtract
			sel = 0x06;
		cTestX = cTemp;
		cTestY = cTemp&0x0F;
		// prepare carry?
		cFlag = mRegMAIN[I8085_REG_F].GetData() & I8085_FLAG_C;
		if(sel&0x02) // sub
		{
			cTestX -= (aword)aData+(sel&cFlag);
			cTestY -= (aword)(aData&0x0F)+(sel&cFlag); // get auxc
		}
		else // add
		{
			cTestX += (aword)aData+(sel&cFlag);
			cTestY += (aword)(aData&0x0F)+(sel&cFlag); // get auxc
		}
		// check c & ac flags
		cFlag = 0x00;
		if(cTestX&0x100) // carry
			cFlag |= I8085_FLAG_C;
		if(cTestY&0x10) // aux carry
			cFlag |= I8085_FLAG_A;
		cTemp = (cTestX&0xFF);
		if(!(sel&0x04)) // cmp doesn't update accumulator!
			mRegMAIN[I8085_REG_A].SetData(cTemp);
	}
	// update flag!
	this->UpdateFlag(cFlag,cTemp);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecDAD(abyte sel)
{
	mRegPAIR[I8085_RP_HL].Accumulate(mRegPAIR[sel].GetData());
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecLXI(abyte sel, aword aData)
{
	mRegPAIR[sel].SetData(aData);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecSTAXLDAX(abyte sel)
{
	this->ExecSTALDA(sel, mRegPAIR[(sel&0x02)>>1].GetData());
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecSTALDA(abyte sel, aword anAddr)
{
	abyte cData;
	// do the transfer!
	if(sel&0x01)
	{
		mErrorRW |= !mMemoryMap.Read(anAddr,cData);
		mRegMAIN[I8085_REG_A].SetData(cData);
	}
	else
	{
		cData = mRegMAIN[I8085_REG_A].GetData();
		mErrorRW |= !mMemoryMap.Write(anAddr,cData);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecSLHLD(abyte sel, aword anAddr)
{
	abyte cData;
	// do the transfer!
	if(sel&0x01)
	{
		mErrorRW |= !mMemoryMap.Read(anAddr++,cData);
		mRegMAIN[I8085_REG_L].SetData(cData);
		mErrorRW |= !mMemoryMap.Read(anAddr,cData);
		mRegMAIN[I8085_REG_H].SetData(cData);
	}
	else
	{
		cData = mRegMAIN[I8085_REG_L].GetData();
		mErrorRW |= !mMemoryMap.Write(anAddr++,cData);
		cData = mRegMAIN[I8085_REG_H].GetData();
		mErrorRW |= !mMemoryMap.Write(anAddr,cData);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecINXDCX(abyte sel)
{
	if(sel&0x01)
		mRegPAIR[(sel&0X06)>>1].Decrement();
	else
		mRegPAIR[(sel&0X06)>>1].Increment();
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecINRDCR(abyte sel, abyte reg)
{
	abyte cData, cFlag;
	aword cTestX = this->GetSrcData(reg);
	aword cTestY = cTestX&0x000F;
	if(sel&0x01) { cTestX--; cTestY--; }
	else { cTestX++; cTestY++; }
	// check carry flags
	cFlag = 0x00;
	if(cTestX&0x100) // carry
		cFlag |= I8085_FLAG_C;
	if(cTestY&0x10) // aux carry
		cFlag |= I8085_FLAG_A;
	// update result
	cData = cTestX&0xFF;
	this->UpdateFlag(cFlag,cData);
	// write-back!
	this->PutDstData(reg,cData);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecROTATE(abyte sel)
{
	abyte cDataA = mRegMAIN[I8085_REG_A].GetData();
	abyte cDataF = mRegMAIN[I8085_REG_F].GetData();
	abyte cTempC = cDataF&0x01, cTempF;
	if(sel&0x01) // rotate right
	{
		cTempF = cDataA&0x01; // value going into carry!
		if((sel&0x02)==0x00) cTempC = cTempF;
		mRegMAIN[I8085_REG_A].SetData((cDataA>>1)|(cTempC<<7));
	}
	else // rotate left
	{
		cTempF = (cDataA&0x80)>>7; // value going into carry!
		if((sel&0x02)==0x00) cTempC = cTempF;
		mRegMAIN[I8085_REG_A].SetData((cDataA<<1)|(cTempC));
	}
	// update carry flag
	if(cTempF) mRegMAIN[I8085_REG_F].SetData(cDataF|I8085_FLAG_C);
	else mRegMAIN[I8085_REG_F].SetData(cDataF&~I8085_FLAG_C);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecDCSC(abyte sel)
{
	abyte cDataA = mRegMAIN[I8085_REG_A].GetData();
	abyte cDataF = mRegMAIN[I8085_REG_F].GetData();
	if(sel&0x02) // carry flag ops
	{
		// complement or set carry flag
		if(sel&0x01) mRegMAIN[I8085_REG_F].SetData(cDataF^I8085_FLAG_C);
		else mRegMAIN[I8085_REG_F].SetData(cDataF|I8085_FLAG_C);
	}
	else // accumulator ops
	{
		if(sel&0x01)
		{
			mRegMAIN[I8085_REG_A].SetData(~cDataA);  // complement accumulator
		}
		else
		{
			// DAA operation
			abyte cData, cFlag = 0x00;
			aword cTestX, cTestY;
			cTestX = cDataA&0x00F0;
			cTestY = cDataA&0x000F;
			// check lower nibble
			if(cTestY>0x09||(cDataF&0x10))
			{
				cTestY += 0x06;
				if(cTestY&0x0010) cFlag |= I8085_FLAG_A;
				cTestY &= 0x0F;
			}
			// check upper nibble
			cTestX += cFlag;
			if(cTestX>0x90||(cDataF&0x01))
			{
				cTestX += 0x60;
				if(cTestX&0x100) cFlag |= I8085_FLAG_C;
				cTestX &= 0xF0;
			}
			// update result
			cData = (cTestX+cTestY)&0xFF;
			this->UpdateFlag(cFlag,cData);
			mRegMAIN[I8085_REG_A].SetData(cData);
		}
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecRSIM(abyte sel)
{
	// SID/SOD NOT IMPLEMENTED YET!
	if(sel)
	{ // sim
		abyte cTest = mRegMAIN[I8085_REG_A].GetData();
		if(cTest&I8085_IMSK_ENB) // set new mask
		{
			abyte cData = mRegINTR.GetData() & ~I8085_IMSK_ALL;
			mRegINTR.SetData(cData|(cTest&I8085_IMSK_ALL));
		}
		if(cTest&I8085_I7P5_RST) // reset flip-flop for rst7.5
			mFlagI7P5 = false;
	}
	else
	{ // rim
		abyte cTest = mRegINTR.GetData() & ~I8085_INTR_ENB;
		if(mIEnabled) cTest |= I8085_INTR_ENB;
		// get interrupt status?
		mRegMAIN[I8085_REG_A].SetData(cTest);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecPUSH(abyte sel)
{
	my1Reg85Pair* pReg16 = &mRegPAIR[sel];
	if(sel==I8085_RP_SP) pReg16 = &mRegPSW;
	aword cData = pReg16->GetData();
	this->DoStackPush(&cData);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecPOP(abyte sel)
{
	aword cData;
	this->DoStackPop(&cData);
	my1Reg85Pair* pReg16 = &mRegPAIR[sel];
	if(sel==I8085_RP_SP) pReg16 = &mRegPSW;
	pReg16->SetData(cData);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecCALL(aword anAddress)
{
	aword cData = mRegPC.GetData();
	this->DoStackPush(&cData);
	mRegPC.SetData(anAddress);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecRET(void)
{
	aword cData;
	this->DoStackPop(&cData);
	mRegPC.SetData(cData);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecRSTn(abyte sel)
{
	aword cTarget = sel * 8;
	this->ExecCALL(cTarget);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecJMP(aword anAddress)
{
	mRegPC.SetData(anAddress);
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecOUTIN(abyte sel, abyte anAddress)
{
	abyte cData;
	if(sel&0x01)
	{
		mDeviceMap.Read(anAddress,cData);
		mRegMAIN[I8085_REG_A].SetData(cData);
	}
	else
	{
		cData = mRegMAIN[I8085_REG_A].GetData();
		mDeviceMap.Write(anAddress,cData);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecCHG(abyte sel)
{
	if(sel&0x01) // xchg
	{
		// swap DE and HL
		my1Reg85Pair cTemp = mRegPAIR[I8085_RP_DE];
		mRegPAIR[I8085_RP_DE] = mRegPAIR[I8085_RP_HL];
		mRegPAIR[I8085_RP_HL] = cTemp;
	}
	else
	{
		aword cTemp, cData;
		this->DoStackPop(&cTemp);
		cData = mRegPAIR[I8085_RP_HL].GetData();
		this->DoStackPush(&cData);
		mRegPAIR[I8085_RP_HL].SetData(cTemp);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecDIEI(abyte sel)
{
	mIEnabled = (sel&0x01) ? true : false;
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecPCSPHL(abyte sel)
{
	if(sel&0x01) // hl to sp
		mRegPAIR[I8085_RP_SP] = mRegPAIR[I8085_RP_HL];
	else // hl to pc
		mRegPC = mRegPAIR[I8085_RP_HL];
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecDelay(int aCount)
{
	if(DoDelay)
		(*DoDelay)((void*)this,aCount);
}
//------------------------------------------------------------------------------
int my1Sim8085::ExecCode(CODEX* pCodex)
{
	// SHOULD CHANGE THIS TO READ FROM MEMORY! USE REG_PC!
	// - issue: if NOT using codex, how to sync with line?
	// - for now, taken care by my1sim85::getcodex
	// reset error flag(s)
	mErrorRW = false;
	mErrorISA = false;
	// check machine state count
	int cStateCount = 0;
	// check halt state
	if(mHalted)
	{
		cStateCount++;
		if(this->CheckInterrupt())
		{
			cStateCount += 4; // BUS IDLE machine cycle! ack rst/trap!
			mHalted = false; // outta here!
		}
		return cStateCount;
	}
	// update program counter? do this here!
	mRegPC.Accumulate(pCodex->size);
	// check opcode!
	if((pCodex->data[0]&0xC0)==0x40) // MOV group
	{
		if((pCodex->data[0]&0x3F)==0x36) // check for HALT!
		{
			mHalted = true;
			cStateCount = 5;
			this->ExecDelay(cStateCount);
			// need to revert PC increment!
			mRegPC.Accumulate(-pCodex->size);
		}
		else
		{
			cStateCount = 4;
			if((pCodex->data[0]&0x38)==0x30||(pCodex->data[0]&0x07)==0x06)
				cStateCount += 3;
			this->ExecDelay(cStateCount);
			this->ExecMOV((pCodex->data[0]&0x38)>>3,(pCodex->data[0]&0x07));
		}
	}
	else if((pCodex->data[0]&0xC0)==0x80) // ALU group
	{
		cStateCount = 4;
		if((pCodex->data[0]&0x07)==0x06)
			cStateCount+=3;
		this->ExecDelay(cStateCount);
		this->ExecALU((pCodex->data[0]&0x38)>>3,(pCodex->data[0]&0x07));
	}
	else if((pCodex->data[0]&0xC0)==0x00) // data proc group
	{
		if((pCodex->data[0]&0x07)==0x06) // mvi
		{
			cStateCount = 7;
			this->ExecDelay(cStateCount);
			this->ExecMOVi((pCodex->data[0]&0x38)>>3,pCodex->data[1]);
		}
		else if((pCodex->data[0]&0x0F)==0x01) // lxi
		{
			aword *pdata = (aword*) &pCodex->data[1];
			cStateCount = 10;
			this->ExecDelay(cStateCount);
			this->ExecLXI((pCodex->data[0]&0x30)>>4,*pdata);
		}
		else if((pCodex->data[0]&0x0F)==0x09) // dad
		{
			cStateCount = 10;
			this->ExecDelay(cStateCount);
			this->ExecDAD((pCodex->data[0]&0x30)>>4);
		}
		else if((pCodex->data[0]&0x27)==0x02) // stax/ldax
		{
			cStateCount = 7;
			this->ExecDelay(cStateCount);
			this->ExecSTAXLDAX((pCodex->data[0]&0x18)>>3);
		}
		else if((pCodex->data[0]&0x37)==0x22) // shld/lhld
		{
			aword *pdata = (aword*) &pCodex->data[1];
			cStateCount = 16;
			this->ExecDelay(cStateCount);
			this->ExecSLHLD((pCodex->data[0]&0x08)>>3,*pdata);
		}
		else if((pCodex->data[0]&0x37)==0x32) // sta/lda
		{
			aword *pdata = (aword*) &pCodex->data[1];
			cStateCount = 13;
			this->ExecDelay(cStateCount);
			this->ExecSTALDA((pCodex->data[0]&0x08)>>3,*pdata);
		}
		else if((pCodex->data[0]&0x07)==0x03) // inx/dcx
		{
			cStateCount = 6;
			this->ExecDelay(cStateCount);
			this->ExecINXDCX((pCodex->data[0]&0x38)>>3);
		}
		else if((pCodex->data[0]&0x06)==0x04) // inr/dcr
		{
			cStateCount = 4;
			if((pCodex->data[0]&0x38)==0x30)
				cStateCount+=6;
			this->ExecDelay(cStateCount);
			this->ExecINRDCR((pCodex->data[0]&0x01),(pCodex->data[0]&0x38)>>3);
		}
		else if((pCodex->data[0]&0x27)==0x07) // rotates
		{
			cStateCount = 4;
			this->ExecDelay(cStateCount);
			this->ExecROTATE((pCodex->data[0]&0x18)>>3);
		}
		else if((pCodex->data[0]&0x27)==0x27) // misc - daa, cma, stc, cmc
		{
			cStateCount = 4;
			this->ExecDelay(cStateCount);
			this->ExecDCSC((pCodex->data[0]&0x18)>>3);
		}
		else if((pCodex->data[0]&0x2F)==0x20) // rim, sim
		{
			cStateCount = 4;
			this->ExecDelay(cStateCount);
			this->ExecRSIM((pCodex->data[0]&0x10)>>4);
		}
		else if(pCodex->data[0]==0x00) // nop
		{
			cStateCount = 4;
			this->ExecDelay(cStateCount);
		}
		else // unspecified instructions (0x08, 0x10, 0x18, 0x28, 0x38)
		{
			mErrorISA = true;
		}
	}
	else if((pCodex->data[0]&0xC0)==0xC0) // control group
	{
		if((pCodex->data[0]&0x07)==0x06) // alu_i group
		{
			cStateCount = 7;
			this->ExecDelay(cStateCount);
			this->ExecALUi((pCodex->data[0]&0x38)>>3,pCodex->data[1]);
		}
		else if((pCodex->data[0]&0x0b)==0x01) // push/pop
		{
			cStateCount = 10;
			if((pCodex->data[0]&0x04))
				cStateCount += 2;
			this->ExecDelay(cStateCount);
			if((pCodex->data[0]&0x04))
				this->ExecPUSH((pCodex->data[0]&0x30)>>4);
			else
				this->ExecPOP((pCodex->data[0]&0x30)>>4);
		}
		else if((pCodex->data[0]&0x3b)==0x09) // call/ret
		{
			aword *pdata = (aword*) &pCodex->data[1];
			cStateCount = 10;
			if((pCodex->data[0]&0x04))
				cStateCount += 8;
			this->ExecDelay(cStateCount);
			if((pCodex->data[0]&0x04))
				this->ExecCALL(*pdata);
			else
				this->ExecRET();
		}
		else if((pCodex->data[0]&0x07)==0x04) // call conditional
		{
			aword *pdata = (aword*) &pCodex->data[1];
			bool cDoThis = this->CheckFlag((pCodex->data[0]&0x38)>>3);
			cStateCount = 9;
			if(cDoThis)
				cStateCount += 9;
			this->ExecDelay(cStateCount);
			if(cDoThis)
				this->ExecCALL(*pdata);
		}
		else if((pCodex->data[0]&0x07)==0x00) // return conditional
		{
			bool cDoThis = this->CheckFlag((pCodex->data[0]&0x38)>>3);
			cStateCount = 6;
			if(cDoThis)
				cStateCount += 6;
			this->ExecDelay(cStateCount);
			if(cDoThis)
				this->ExecRET();
		}
		else if((pCodex->data[0]&0x07)==0x07) // rst n
		{
			cStateCount = 12;
			this->ExecDelay(cStateCount);
			this->ExecRSTn((pCodex->data[0]&0x38)>>3);
		}
		else if((pCodex->data[0]&0x3F)==0x03) // jmp
		{
			aword *pdata = (aword*) &pCodex->data[1];
			cStateCount = 10;
			this->ExecDelay(cStateCount);
			this->ExecJMP(*pdata);
		}
		else if((pCodex->data[0]&0x07)==0x02) // jump conditional
		{
			aword *pdata = (aword*) &pCodex->data[1];
			bool cDoThis = this->CheckFlag((pCodex->data[0]&0x38)>>3);
			cStateCount = 7;
			if(cDoThis)
				cStateCount += 3;
			this->ExecDelay(cStateCount);
			if(cDoThis)
				this->ExecJMP(*pdata);
		}
		else if((pCodex->data[0]&0x37)==0x13) // out/in
		{
			cStateCount = 10;
			this->ExecDelay(cStateCount);
			this->ExecOUTIN((pCodex->data[0]&0x08)>>3, pCodex->data[1]);
		}
		else if((pCodex->data[0]&0x37)==0x23) // xthl/xchg
		{
			cStateCount = 4;
			if(!(pCodex->data[0]&0x08))
				cStateCount += 16;
			this->ExecDelay(cStateCount);
			this->ExecCHG((pCodex->data[0]&0x08)>>3);
		}
		else if((pCodex->data[0]&0x37)==0x33) // di/ei
		{
			cStateCount = 4;
			this->ExecDelay(cStateCount);
			this->ExecDIEI((pCodex->data[0]&0x08)>>3);
		}
		else if((pCodex->data[0]&0x29)==0x29) // pchl/sphl
		{
			cStateCount = 4;
			this->ExecDelay(cStateCount);
			this->ExecPCSPHL((pCodex->data[0]&0x10)>>4);
		}
		else // unspecified instructions (0xcb, 0xd9, 0xdd, 0xed, 0xfd)
		{
			mErrorISA = true;
		}
	}
	// only check interrupt if exec okay?
	if(cStateCount)
	{
		if(this->CheckInterrupt())
			cStateCount += 4; // BUS IDLE machine cycle! ack rst/trap!
	}
	return cStateCount;
}
//------------------------------------------------------------------------------
my1BitIO& my1Sim8085::Pin(int anIndex)
{
	return mPins[anIndex];
}
//------------------------------------------------------------------------------
my1MemoryMap85& my1Sim8085::MemoryMap(void)
{
	return mMemoryMap;
}
//------------------------------------------------------------------------------
my1DeviceMap85& my1Sim8085::DeviceMap(void)
{
	return mDeviceMap;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Sim85::my1Sim85()
{
	mReady = false; mBuilt = false; mBegan = false;
	mStartAddress = 0x0000;
	mCodeLink = 0x0;
	mCodeCount = 0; mStatePrev = 0; mStateTotal = 0;
	mCodexList = 0x0; mCodexExec = 0x0;
	mCodexPrev = 0x0; mCodexNone = 0x0;
}
//------------------------------------------------------------------------------
my1Sim85::~my1Sim85()
{
	this->BuildReset();
	this->FreeCodex();
	mCodexNone = free_codex(mCodexNone);
}
//------------------------------------------------------------------------------
bool my1Sim85::Ready(void)
{
	return mReady;
}
//------------------------------------------------------------------------------
bool my1Sim85::Built(void)
{
	return mBuilt;
}
//------------------------------------------------------------------------------
bool my1Sim85::Halted(void)
{
	return mHalted;
}
//------------------------------------------------------------------------------
bool my1Sim85::Interrupted(void)
{
	return mExecINTR;
}
//------------------------------------------------------------------------------
bool my1Sim85::NoCodex(void)
{
	return mCodexNone;
}
//------------------------------------------------------------------------------
int my1Sim85::GetStartAddress(void)
{
	return mStartAddress;
}
//------------------------------------------------------------------------------
void my1Sim85::SetStartAddress(int anAddress)
{
	if(anAddress<0) anAddress = 0;
	else if(anAddress>MAX_MEMSIZE-1) anAddress = MAX_MEMSIZE-1;
	mStartAddress = anAddress;
}
//------------------------------------------------------------------------------
void* my1Sim85::GetCodeLink(void)
{
	return mCodeLink;
}
//------------------------------------------------------------------------------
void my1Sim85::SetCodeLink(void* aCodeLink)
{
	mCodeLink = aCodeLink;
}
//------------------------------------------------------------------------------
bool my1Sim85::FreeCodex(void)
{
	bool cFlag = true;
	while(mCodexList)
	{
		mCodexExec = mCodexList;
		mCodexList = mCodexList->next;
		free_codex(mCodexExec);
		mCodeCount--;
	}
	mCodexExec = 0x0;
	mCodexPrev = 0x0;
	if(mCodeCount>0)
	{
		mCodeCount = 0;
		cFlag = false;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::LoadCodex(char *aFilename)
{
	// following c-style coding (originate from my1i8085!)
	// initialize main data structure
	STUFFS things;
	initialize(&things);
	things.afile = aFilename;
	// try to redirect stdout
#ifdef DO_MINGW
#define TEMP_FILENAME "temp.txt"
	FILE *pFile = fopen(TEMP_FILENAME,"wt");
#else
	char *pBuffer = 0x0;
	size_t cSize = 0x0;
	FILE *pFile = open_memstream(&pBuffer, &cSize);
#endif
	if(!pFile) return false;
	things.opt_stdout = pFile;
	things.opt_stderr = pFile;
	fflush(pFile);
	// print tool info
	fprintf(pFile,"\n%s - 8085 Assembler\n", PROGNAME);
	fprintf(pFile,"  => by azman@my1matrix.net\n\n");
	fflush(pFile);
	// false do-while loop - for error exits
	do
	{
		// first pass - build label db
		things.pass = EXEC_PASS_1;
		if(process_asmfile(&things)>0)
			break;
		// second pass - arrange code
		things.pass = EXEC_PASS_2;
		if(process_asmfile(&things)>0)
			break;
	}
	while(0);
	// print end indicator
	fprintf(pFile,"\n%s - Done!\n\n", PROGNAME);
	if(!things.errc)
	{
		do // error exit route!
		{
			// copy codex to local
			if(!this->FreeCodex()) { things.errc++; break; }
			mCodexList = things.pcodex;
			if(!mCodexList) { things.errc++; break; }
			things.pcodex = 0x0; // we'll take it from here!
		}
		while(0);
	}
	fclose(pFile);
	// clean-up redirect stuffs
	things.opt_stdout = stdout;
	things.opt_stderr = stdout;
	// send out the output!
#ifdef DO_MINGW
	std::fstream infile;
	infile.open(TEMP_FILENAME,std::fstream::in);
	if(infile.is_open())
	{
		while(!infile.eof())
		{
			int test = infile.get();
			if(test<0) break;
			std::cout << (char) test;
		}
		infile.close();
		remove(TEMP_FILENAME);
	}
#else
	std::cout << pBuffer;
	free(pBuffer);
#endif
	// clean-up main data structure
	cleanup(&things);
	return things.errc ? false : true; // still maintained in structure
}
//------------------------------------------------------------------------------
bool my1Sim85::MEMCodex(void)
{
	// check if codex is loaded?
	CODEX *pCodex = mCodexList;
	if(!pCodex) return false;
	int cError = 0;
	// output message
	std::cout << "Loading code to simulator system memory... ";
	// set program mode
	mMemoryMap.ProgramMode();
	// start browsing codes
	do
	{
		/* fill memory with codex data */
		for(int cLoop=0;cLoop<pCodex->size;cLoop++)
		{
			if(!mMemoryMap.Write(pCodex->addr+cLoop,pCodex->data[cLoop]))
			{
				/* invalid data location? */
				std::cout << "MEMORY DATA ERROR: ";
				std::cout << "CodexAddr="
					<< my1ValueHEX(pCodex->addr,4) << "H, ";
				std::cout << "CodexData=";
				for(int cIndex=0;cIndex<pCodex->size;cIndex++)
					std::cout << my1ValueHEX(pCodex->data[cIndex],2) << "H, ";
				std::cout << "CodexCount=" << mCodeCount << "\n";
				cError++;
			}
		}
		pCodex = pCodex->next;
	}
	while(pCodex);
	// unset program mode
	mMemoryMap.ProgramMode(false);
	// output results
	if(!cError)
		std::cout << "done!\n\n";
	else
		std::cout << "error! (" << cError << ")\n\n";
	return cError > 0 ? false : true;
}
//------------------------------------------------------------------------------
bool my1Sim85::HEXCodex(char* aFilename)
{
	// output message
	std::cout << "Writing HEX file... ";
	int cError = generate_hex(mCodexList,aFilename);
	if(!cError)
		std::cout << "done!\n\n";
	else
		std::cout << "error! (" << cError << ")\n\n";
	return  cError > 0 ? false : true;
}
//------------------------------------------------------------------------------
bool my1Sim85::GetCodex(aword anAddress)
{
	bool cFlag = false;
	if(MemoryMap().Memory(anAddress))
	{
		CODEX *pcodex = mCodexList;
		while(pcodex)
		{
			// check address and instruction flag!
			if(pcodex->addr==anAddress&&pcodex->line)
			{
				cFlag = true; // assume true, check data mismatch!
				for(int cLoop=0;cLoop<pcodex->size;cLoop++)
				{
					abyte cData = 0xFF;
					my1Memory* pMemory = MemoryMap().Memory(anAddress+cLoop);
					if(!pMemory->GetData(anAddress+cLoop,cData)||
						cData!=pcodex->data[cLoop])
					{
						cFlag = false;
						std::cout << "[ERROR] ";
						std::cout << "Address: 0x"
							<< my1ValueHEX(anAddress+cLoop,4) << ", ";
						std::cout << "Codex Data: 0x"
							<< my1ValueHEX(pcodex->data[cLoop],2) << ", ";
						std::cout << "Memory Data: 0x"
							<< my1ValueHEX(cData,2) << "!\n";
						break;
					}
				}
				if(cFlag)
					mCodexExec = pcodex;
				break;
			}
			pcodex = pcodex->next;
		}
		if(!cFlag)
		{
			if(!mCodexNone)
			{
				mCodexNone = create_codex(1);
				mCodexNone->data[0] = I8085_HALT_CODE;
			}
			mCodexExec = mCodexNone;
			cFlag = true;
		}
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::ExeCodex(void)
{
	if(!mBuilt)
	{
		std::cout << "[SIM Error] System Build Incomplete!\n" ;
		return false;
	}
	if(!mCodexExec)
	{
		std::cout << "[SIM Error] Cannot get codex to execute!\n" ;
		return false;
	}
	mStatePrev = this->ExecCode(mCodexExec);
	mCodexPrev = mCodexExec;
	bool cExecOK = !(mErrorISA || mErrorRW || !mStatePrev);
	if(cExecOK)
	{
		mStateTotal += mStatePrev;
	}
	else
	{
		if(mErrorISA)
			std::cout << "[ISA Error] Invalid Instruction!\n" ;
		if(mErrorRW)
			std::cout << "[R/W Error] Invalid Memory Read/Write!\n" ;
		this->PrintCodexInfo(mCodexExec);
	}
	return cExecOK;
}
//------------------------------------------------------------------------------
bool my1Sim85::ResetSim(int aStart)
{
	this->Reset();
	mStatePrev = 0;
	mStateTotal = 0;
	mCodexPrev = 0x0;
	mCodexNone = free_codex(mCodexNone);
	mRegPC.SetData((aword) aStart);
	if(DoUpdate)
		(*DoUpdate)((void*)this);
	return this->GetCodex(mRegPC.GetData());
}
//------------------------------------------------------------------------------
bool my1Sim85::StepSim(void)
{
	if(!this->ExeCodex())
		return false;
	if(DoUpdate)
		(*DoUpdate)((void*)this);
	return this->GetCodex(mRegPC.GetData());
}
//------------------------------------------------------------------------------
bool my1Sim85::RunSim(int aStep)
{
	bool cFlag = true;
	bool cFlow = true;
	while(cFlag&&cFlow)
	{
		cFlag = this->StepSim();
		if(aStep<0)
			cFlow = !mHalted;
		else if(--aStep==0)
			cFlow = false;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::BuildDefault(void)
{
	if(mBuilt) this->BuildReset();
	if(!this->AddROM()) return false;
	if(!this->AddRAM()) return false;
	if(!this->AddPPI()) return false;
	return true;
}
//------------------------------------------------------------------------------
bool my1Sim85::BuildReset(void)
{
	while(mMemoryMap.GetCount()>0)
	{
		my1Memory* pMemory = (my1Memory*) mMemoryMap.Remove();
		delete pMemory;
	}
	mBuilt = false; // no memory?
	while(mDeviceMap.GetCount()>0)
	{
		my1Device* pDevice = (my1Device*) mDeviceMap.Remove();
		delete pDevice;
	}
	// reset pin do_update and do_detect functions as well??
	return true;
}
//------------------------------------------------------------------------------
bool my1Sim85::AddROM(int aStart)
{
	bool cFlag = false;
	my1Sim2764 *pROM = new my1Sim2764(aStart);
	if(mMemoryMap.Insert((my1Address*)pROM))
	{
		cFlag = true;
		mBuilt = true;
	}
	else
	{
		delete pROM; // assume new malloc always successful!
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::AddRAM(int aStart)
{
	bool cFlag = false;
	my1Sim6264 *pRAM = new my1Sim6264(aStart);
	if(mMemoryMap.Insert((my1Address*)pRAM))
	{
		cFlag = true;
		mBuilt = true;
	}
	else
	{
		delete pRAM; // assume new malloc always successful!
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::AddPPI(int aStart)
{
	bool cFlag = false;
	my1Sim8255 *pPPI = new my1Sim8255(aStart);
	if(mDeviceMap.Insert((my1Address*)pPPI))
	{
		cFlag = true;
	}
	else
	{
		delete pPPI; // assume new malloc always successful!
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::BuildLoad(const char* aFileName)
{
	return false;
}
//------------------------------------------------------------------------------
bool my1Sim85::BuildSave(const char* aFileName)
{
	return false;
}
//------------------------------------------------------------------------------
bool my1Sim85::Assemble(const char* aFileName)
{
	mBegan = false;
	mReady = this->LoadCodex((char*)aFileName);
	if(!mReady) std::cout << "[ERROR] Assemble Failed!\n" ;
#ifdef MY1DEBUG
	this->PrintCodexInfo();
#endif
	return mReady;
}
//------------------------------------------------------------------------------
bool my1Sim85::Generate(const char* aFileName)
{
	if(!mReady)
	{
		std::cout << "[ERROR] Cannot generate - run assembler!\n";
		return false;
	}
	return this->HEXCodex((char*)aFileName);
}
//------------------------------------------------------------------------------
bool my1Sim85::Simulate(int aStep)
{
	if(!mBuilt)
	{
		std::cout << "[ERROR] Cannot simulate with incomplete system!\n";
		return false;
	}
	if(!mReady)
	{
		std::cout << "[ERROR] Cannot simulate - run assembler!\n";
		return false;
	}
	if(!mBegan||aStep<1)
	{
		if(!this->MEMCodex())
		{
			std::cout << "[ERROR] Cannot load code to memory!\n" ;
			return false;
		}
		mReady = this->ResetSim(mStartAddress);
		if(!mReady)
			std::cout << "[ERROR] Cannot fetch code from memory!\n" ;
		mBegan = mReady;
	}
	else
	{
		mReady = this->RunSim(aStep);
		if(!mReady)
			std::cout << "[ERROR] Simulation error!\n" ;
	}
#ifdef MY1DEBUG
	this->PrintCodexInfo();
#endif
	return mReady;
}
//------------------------------------------------------------------------------
my1Reg85* my1Sim85::Register(int anIndex)
{
	my1Reg85* pReg85 = 0x0;
	if(anIndex/I8085_REG_COUNT)
	{
		switch(anIndex%I8085_REG_COUNT)
		{
			case I8085_RP_PC:
				pReg85 = &mRegPC;
				break;
			case I8085_RP_SP:
				pReg85 = &mRegPAIR[I8085_RP_SP];
				break;
		}
	}
	else
	{
		pReg85 = &mRegMAIN[anIndex%I8085_REG_COUNT];
	}
	return pReg85;
}
//------------------------------------------------------------------------------
my1Memory* my1Sim85::Memory(int anIndex)
{
	return (my1Memory*) mMemoryMap.Object(anIndex);
}
//------------------------------------------------------------------------------
my1Device* my1Sim85::Device(int anIndex)
{
	return (my1Device*) mDeviceMap.Object(anIndex);
}
//------------------------------------------------------------------------------
int my1Sim85::GetCodexLine(void)
{
	return mCodexExec ? mCodexExec->line : 0;
}
//------------------------------------------------------------------------------
void my1Sim85::PrintCodexInfo(CODEX* aCodex)
{
	if(mReady||aCodex)
	{
		if(!aCodex)
			aCodex = mCodexExec;
		std::cout << "[Codex Info] Addr: "
			<< my1ValueHEX(aCodex->addr,4) << ", ";
		std::cout << "Line: "
			<< my1ValueDEC(aCodex->line) << ", ";
		std::cout << "Data: ";
		for(int cLoop=0;cLoop<aCodex->size;cLoop++)
			std::cout << my1ValueHEX(aCodex->data[cLoop],2) << ",";
		std::cout << " T-States: ";
		if(aCodex==mCodexPrev)
			std::cout << my1ValueDEC(mStatePrev);
		else
			std::cout << "Waiting execution!";
		std::cout << ", Total T-States: " << my1ValueDEC(mStateTotal) << "\n";
	}
}
//------------------------------------------------------------------------------
void my1Sim85::PrintCodexPrev(void)
{
	if(mCodexPrev)
		this->PrintCodexInfo(mCodexPrev);
	else
		std::cout << "[Codex Info] No previous execution!" << std::endl;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
