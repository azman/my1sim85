//------------------------------------------------------------------------------
#include "my1sim85.hpp"
//------------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
//------------------------------------------------------------------------------
#define PROGNAME "my1sim85"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1SimObject::my1SimObject()
{
	mName[0] = 0x0;
	mLink = 0x0;
	DoUpdate = 0x0;
	DoDetect = 0x0;
	DoDelay = 0x0;
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
//------------------------------------------------------------------------------
my1Address::my1Address(int aStart, int aSize)
{
	if(aStart<0) mStart = 0;
	else mStart = aStart;
	if(aSize>0&&aSize<=MAX_MEMSIZE) mSize = aSize;
	else mSize = 0;
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
//------------------------------------------------------------------------------
my1Memory::my1Memory(int aStart, int aSize, bool aROM)
	: my1Address(aStart,aSize)
{
	mReadOnly = aROM;
	mProgramMode = false;
	// mLastUsed = 0x0; // let random?
	if(mSize>0) mSpace = new abyte[mSize];
	else mSpace = 0x0;
}
//------------------------------------------------------------------------------
my1Memory::~my1Memory()
{
	if(mSpace) delete mSpace;
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
bool my1Memory::ReadData(aword anAddress, abyte& rData)
{
	if(!this->IsSelected(anAddress)) return false;
	mLastUsed = anAddress-mStart;
	rData = mSpace[mLastUsed];
	if(DoDelay) // example to implement access delay!
		(*DoDelay)((void*)this);
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
	// mState is random?
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
my1DevicePort::my1DevicePort(int aSize)
{
	mSize = 0;
	mDevicePins = 0x0;
	if(aSize>0) this->SetSize(aSize);
}
//------------------------------------------------------------------------------
my1DevicePort::~my1DevicePort()
{
	if(mSize) delete mDevicePins;
}
//------------------------------------------------------------------------------
int my1DevicePort::GetSize(void)
{
	return mSize;
}
//------------------------------------------------------------------------------
void my1DevicePort::SetSize(int aSize)
{
	if(mSize>0) return; // 1-time setting!
	if(aSize>MAX_PORTPIN_COUNT) return; // check max bit-size
	mSize = aSize;
	mDevicePins = new my1BitIO[mSize];
}
//------------------------------------------------------------------------------
my1BitIO* my1DevicePort::GetBitIO(int anIndex)
{
	return &mDevicePins[anIndex];
}
//------------------------------------------------------------------------------
abyte my1DevicePort::IsInput(void)
{
	abyte cFlag = 0x00, cMask = 0x01;
	for(int cLoop=0;cLoop<mSize;cLoop++)
	{
		if(mDevicePins[cLoop].IsInput())
			cFlag |= cMask;
		cMask <<= 1;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
void my1DevicePort::SetInput(bool anInput)
{
	for(int cLoop=0;cLoop<mSize;cLoop++)
		mDevicePins[cLoop].SetInput(anInput);
}
//------------------------------------------------------------------------------
abyte my1DevicePort::GetPort(void)
{
	abyte cData = 0x0, cMask = 0x01;
	for(int cLoop=0;cLoop<mSize;cLoop++)
	{
		if(mDevicePins[cLoop].GetState()==BIT_STATE_1)
			cData |= cMask;
		cMask <<= 1;
	}
	return cData;
}
//------------------------------------------------------------------------------
void my1DevicePort::SetPort(abyte aData)
{
	abyte cMask = 0x01;
	for(int cLoop=0;cLoop<mSize;cLoop++)
	{
		if(aData&cMask)
			mDevicePins[cLoop].SetState(BIT_STATE_1);
		else
			mDevicePins[cLoop].SetState(BIT_STATE_0);
		cMask <<= 1;
	}
}
//------------------------------------------------------------------------------
abyte my1DevicePort::GetData(void)
{
	abyte cData = 0x0, cMask = 0x01;
	if(DoDetect) // either here or in bitio!
		(*DoDetect)((void*)this);
	for(int cLoop=0;cLoop<mSize;cLoop++)
	{
		if(mDevicePins[cLoop].GetData()==BIT_STATE_1)
			cData |= cMask;
		cMask <<= 1;
	}
	return cData;
}
//------------------------------------------------------------------------------
void my1DevicePort::SetData(abyte aData)
{
	abyte cMask = 0x01;
	for(int cLoop=0;cLoop<mSize;cLoop++)
	{
		if(aData&cMask)
			mDevicePins[cLoop].SetData(BIT_STATE_1);
		else
			mDevicePins[cLoop].SetData(BIT_STATE_0);
		cMask <<= 1;
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
	if(mDevicePorts) delete mDevicePorts;
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
	for(int cLoop=0;cLoop<I8255_SIZE;cLoop++)
		mDevicePorts[cLoop].SetSize(I8255_DATASIZE);
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
				// config data - always assume mode 0!
				mDevicePorts[cIndex].SetPort(aData);
				if(aData&0x10) cCheck = true;
				else cCheck = false;
				mDevicePorts[I8255_PORTA].SetInput(cCheck);
				if(aData&0x02) cCheck = true;
				else cCheck = false;
				mDevicePorts[I8255_PORTB].SetInput(cCheck);
				if(aData&0x08) cCheck = true;
				else cCheck = false;
				my1DevicePort* aPort = &mDevicePorts[I8255_PORTC];
				my1BitIO* aPin;
				for(int cLoop=0;cLoop<4;cLoop++)
				{
					aPin = aPort->GetBitIO(cLoop);
					aPin->SetInput(cCheck);
				}
				if(aData&0x01) cCheck = true;
				else cCheck = false;
				for(int cLoop=5;cLoop<8;cLoop++)
				{
					aPin = aPort->GetBitIO(cLoop);
					aPin->SetInput(cCheck);
				}
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
my1Reg85::my1Reg85(bool aDoubleSize)
{
	// mData = 0x00; // let it be random?
	mDoubleSize = aDoubleSize;
	pHI = 0x0; pLO = 0x0;
}
//------------------------------------------------------------------------------
void my1Reg85::Use(my1Reg85* aReg, my1Reg85* bReg)
{
	pHI = aReg; pLO = bReg;
	mDoubleSize = true;
}
//------------------------------------------------------------------------------
bool my1Reg85::IsDoubleSize(void)
{
	return mDoubleSize;
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
		if(DoDetect)
			(*DoDetect)((void*)this);
		if(mDoubleSize) cValue = mData;
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
		pLO->SetData(aData&&0xFF);
	}
	else
	{
		if(mDoubleSize) mData = aData;
		else  mData = aData&0xFF;
		if(DoUpdate)
			(*DoUpdate)((void*)this);
	}
}
//------------------------------------------------------------------------------
aword my1Reg85::Increment(void)
{
	this->SetData(this->GetData()+1);
	return mData;
}
//------------------------------------------------------------------------------
aword my1Reg85::Decrement(void)
{
	this->SetData(this->GetData()-1);
	return mData;
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
my1Pin85::my1Pin85()
	: my1DevicePort(I8085_PIN_COUNT)
{
	mData = 0x00; // clear intr flags!
}
//------------------------------------------------------------------------------
my1BitIO& my1Pin85::RegBit(int anIndex)
{
	return mDevicePins[anIndex];
}
//------------------------------------------------------------------------------
aword my1Pin85::GetData(void)
{
	return my1Reg85::GetData();
}
//------------------------------------------------------------------------------
void my1Pin85::SetData(aword aData)
{
	my1Reg85::SetData(aData);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1AddressMap::my1AddressMap()
{
	for(int cLoop=0;cLoop<MAX_ADDRMAP_COUNT;cLoop++)
		mObjects[cLoop] = 0x0;
	mCount = 0;
	mMapSize = 0;
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
int my1AddressMap::Insert(my1Address* anObject, int anIndex)
{
	int cFirstFree = -1;
	// check if outside total address space!
	int cBegin = anObject->GetStart();
	int cTotal = cBegin + anObject->GetSize();
	if(cBegin<0||cTotal>=mMapSize) return false;
	// check existing address space
	for(int cLoop=0;cLoop<MAX_ADDRMAP_COUNT;cLoop++)
	{
		if(mObjects[cLoop])
		{
			if(anIndex==cLoop||mObjects[cLoop]->IsOverlapped(*anObject))
				return false;
		}
		else
		{
			if(cFirstFree<0)
				cFirstFree = cLoop;
		}
	}
	// check requested index
	if(anIndex<0||anIndex>MAX_ADDRMAP_COUNT-1)
	{
		if(cFirstFree<0) return false;
		anIndex = cFirstFree;
	}
	// okay... insert!
	mObjects[anIndex] = anObject;
	mCount++;
	return true;
}
//------------------------------------------------------------------------------
my1Address* my1AddressMap::Remove(int anIndex)
{
	int cFirst = -1;
	// check existing address space
	for(int cLoop=0;cLoop<MAX_ADDRMAP_COUNT;cLoop++)
	{
		if(mObjects[cLoop])
		{
			if(cFirst<0) cFirst = cLoop;
		}
		else
		{
			if(anIndex==cLoop) return 0x0;
		}
	}
	// check requested index
	if(anIndex<0||anIndex>MAX_ADDRMAP_COUNT-1)
	{
		if(cFirst<0) return 0x0;
		anIndex = cFirst;
	}
	// okay... remove!
	my1Address *cObject = mObjects[anIndex];
	mObjects[anIndex] = 0x0;
	return cObject;
}
//------------------------------------------------------------------------------
my1Address* my1AddressMap::Object(aword anAddress)
{
	// check existing address space
	for(int cLoop=0;cLoop<MAX_ADDRMAP_COUNT;cLoop++)
	{
		if(mObjects[cLoop])
			if(mObjects[cLoop]->IsSelected(anAddress))
				return mObjects[cLoop];
	}
	return 0x0;
}
//------------------------------------------------------------------------------
my1Address* my1AddressMap::Object(int anIndex)
{
	return mObjects[anIndex];
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
	for(int cLoop=0;cLoop<MAX_ADDRMAP_COUNT;cLoop++)
	{
		if(mObjects[cLoop])
		{
			my1Memory* pMem = (my1Memory*) mObjects[cLoop];
			pMem->ProgramMode(aStatus);
		}
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
	mErrorRW = false;
	mErrorISA = false;
	mHalted = false;
	mIEnabled = false;
	// initialize register pairs
	mRegPAIR[I8085_RP_BC].Use(&mRegMAIN[I8085_REG_B],&mRegMAIN[I8085_REG_C]);
	mRegPAIR[I8085_RP_DE].Use(&mRegMAIN[I8085_REG_D],&mRegMAIN[I8085_REG_E]);
	mRegPAIR[I8085_RP_HL].Use(&mRegMAIN[I8085_REG_H],&mRegMAIN[I8085_REG_L]);
	// reset certain registers only
	//mRegINTR.SetData(0x00); // already initialized!
	mRegPC.SetData(0x0000);
	// set input pins
	mRegINTR.RegBit(I8085_PIN_SID).SetInput();
	mRegINTR.RegBit(I8085_PIN_INTR).SetInput();
	mRegINTR.RegBit(I8085_PIN_I7P5).SetInput();
	mRegINTR.RegBit(I8085_PIN_I6P5).SetInput();
	mRegINTR.RegBit(I8085_PIN_I5P5).SetInput();
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
			mErrorRW |= mMemory.Read(mRegPAIR[I8085_RP_HL].GetData(),cData);
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
			mErrorRW |= mMemory.Write(mRegPAIR[I8085_RP_HL].GetData(),aData);
 			break;
		default:
			mRegMAIN[dst].SetData(aData);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::DoStackPush(aword* pData)
{
	abyte *pDataL = (abyte*) pData;
	abyte *pDataH = (abyte*) (pData+1);
	mErrorRW |= mMemory.Write(mRegPAIR[I8085_RP_SP].Decrement(),*pDataH);
	mErrorRW |= mMemory.Write(mRegPAIR[I8085_RP_SP].Decrement(),*pDataL);
}
//------------------------------------------------------------------------------
void my1Sim8085::DoStackPop(aword* pData)
{
	abyte *pDataL = (abyte*) pData;
	abyte *pDataH = (abyte*) (pData+1);
	mErrorRW |= mMemory.Read(mRegPAIR[I8085_RP_SP].Increment(),*pDataH);
	mErrorRW |= mMemory.Read(mRegPAIR[I8085_RP_SP].Increment(),*pDataL);
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
		case 8: cTest = I8085_FLAG_S; break; // jp, jm
	}
	cStatus = (cFlag&cTest) ? false : true;
	if(sel&0x01) cStatus = !cStatus;
	return cStatus;
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
			case 0x10:
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
		if(sel&0x02) // sub
		{
			cTestX -= (aword)aData+(sel&0x01);
			cTestY -= (aword)(aData&0x0F)+(sel&0x01); // get auxc
		}
		else // add
		{
			cTestX += (aword)aData+(sel&0x01);
			cTestY += (aword)(aData&0x0F)+(sel&0x01); // get auxc
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
		mErrorRW |= mMemory.Read(anAddr,cData);
		mRegMAIN[I8085_REG_A].SetData(cData);
	}
	else
	{
		cData = mRegMAIN[I8085_REG_A].GetData();
		mErrorRW |= mMemory.Write(anAddr,cData);
	}
}
//------------------------------------------------------------------------------
void my1Sim8085::ExecSLHLD(abyte sel, aword anAddr)
{
	abyte cData;
	// do the transfer!
	if(sel&0x01)
	{
		mErrorRW |= mMemory.Read(anAddr++,cData);
		mRegMAIN[I8085_REG_L].SetData(cData);
		mErrorRW |= mMemory.Read(anAddr,cData);
		mRegMAIN[I8085_REG_H].SetData(cData);
	}
	else
	{
		cData = mRegMAIN[I8085_REG_L].GetData();
		mErrorRW |= mMemory.Write(anAddr++,cData);
		cData = mRegMAIN[I8085_REG_H].GetData();
		mErrorRW |= mMemory.Write(anAddr,cData);
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
	abyte cTempC;
	abyte cDataA = mRegMAIN[I8085_REG_A].GetData();
	abyte cDataF = mRegMAIN[I8085_REG_F].GetData();
	if(sel&0x01) // rotate right
	{
		if(sel&0x02) cTempC = cDataF&0x01;
		else cTempC = cDataA&0x01;
		mRegMAIN[I8085_REG_A].SetData((cDataA>>1)|(cTempC<<7));
	}
	else // rotate left
	{
		if(sel&0x02) cTempC = cDataF&0x01;
		else cTempC = (cDataA&0x80)>>7;
		mRegMAIN[I8085_REG_A].SetData((cDataA<<1)|(cTempC));
	}
	// update carry flag
	if(cTempC) mRegMAIN[I8085_REG_F].SetData(cDataF|cTempC);
	else mRegMAIN[I8085_REG_F].SetData(cDataF&~cTempC);
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
			}
			// check upper nibble
			cTestX += cFlag;
			if(cTestX>0x90||(cDataF&0x01))
			{
				cTestX += 0x60;
				if(cTestX&0x100) cFlag |= I8085_FLAG_C;
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
	if(sel)
	{ // sim
		mRegINTR.SetData(mRegMAIN[I8085_REG_A].GetData());
	}
	else
	{ // rim
		mRegMAIN[I8085_REG_A].SetData(mRegINTR.GetData());
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
		mDevice.Read(anAddress,cData);
		mRegMAIN[I8085_REG_A].SetData(cData);
	}
	else
	{
		cData = mRegMAIN[I8085_REG_A].GetData();
		mDevice.Write(anAddress,cData);
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
	// check halt state and interrupt request?
	if(mHalted)
		return false;
	if(mIEnabled)
	{
		// what to check?
	}
	// check machine state count
	int cStateCount = 0;
	// reset error flag(s)
	mErrorRW = false;
	mErrorISA = false;
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
		else if((pCodex->data[0]&0x27)==0x17) // misc - daa, cma, stc, cmc
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
			mErrorISA = false;
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
			mErrorISA = false;
		}
	}
	return cStateCount;
}
//------------------------------------------------------------------------------
my1BitIO& my1Sim8085::Pin(int anIndex)
{
	return mRegINTR.RegBit(anIndex);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Sim85::my1Sim85(bool aDefaultConfig)
	: mROM(0x0000), mRAM(0x2000), mPPI(0x80)
{
	mReady = false;
	mStartAddress = 0x0000;
	mCodeCount = 0;
	mCodexList = 0x0; mCodexExec = 0x0;
	if(aDefaultConfig)
	{
		this->BuildDefault();
	}
}
//------------------------------------------------------------------------------
my1Sim85::~my1Sim85()
{
	this->FreeCodex();
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
	// initialize main data structure
	STUFFS things;
	initialize(&things);
	things.afile = aFilename;
	// try to redirect stdout
	char *pBuffer = 0x0;
#ifdef DO_MINGW
	FILE *pFile = stdout;
#else
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
		fprintf(pFile,"Loading code to simulator system memory... ");
		// copy codex to local
		this->FreeCodex();
		this->LoadStuff(&things);
		if(!things.errc)
			fprintf(pFile,"done!\n\n");
		else
			fprintf(pFile,"error! (%d)\n\n",things.errc);
	}
	// clean-up redirect stuffs
	things.opt_stdout = stdout;
	things.opt_stderr = stdout;
	fclose(pFile);
	// send out the output!
	std::cout << pBuffer;
	free(pBuffer);
	// clean-up main data structure
	cleanup(&things);
	return things.errc ? false : true; // still maintained in structure
}
//------------------------------------------------------------------------------
void my1Sim85::LoadStuff(STUFFS* pstuffs)
{
	// following c-style coding (originate from my1i8085!)
	CODEX *pcodex, *tcodex, *ccodex= mCodexList; // should already be 0x0!
	// check if there are any codes?
	pcodex = pstuffs->pcodex;
	if(!pcodex) return;
	pstuffs->addr = pcodex->addr;
	// set program mode
	mMemory.ProgramMode();
	// start browsing codes
	while(pcodex)
	{
		// create new codex to save in class storage
		tcodex = clone_codex(pcodex);
		if(ccodex)
			ccodex->next = tcodex;
		else
			mCodexList = tcodex;
		ccodex = tcodex;
		mCodeCount++;
		/* fill memory with codex data */
		for(int cLoop=0;cLoop<pcodex->size;cLoop++)
		{
			if(!this->WriteMemory(pcodex->addr+cLoop,pcodex->data[cLoop]))
			{
				/* invalid data location? */
				fprintf(pstuffs->opt_stdout,"LOAD DATA ERROR: ");
				fprintf(pstuffs->opt_stdout,"CodexAddr=%04XH, CodexData=%02XH ",
					(pcodex->addr+cLoop), pcodex->data[cLoop]);
				fprintf(pstuffs->opt_stdout,"CodexCount=%d\n", mCodeCount);
				pstuffs->errc++;
			}
		}
		pcodex = pcodex->next;
	}
	// unset program mode
	mMemory.ProgramMode(false);
}
//------------------------------------------------------------------------------
bool my1Sim85::GetCodex(aword anAddress)
{
	bool cFlag = false;
	CODEX *pcodex = mCodexList;
	while(pcodex)
	{
		if(pcodex->addr==anAddress)
		{
			if(pcodex->line) // assigned only if an instruction!
			{
				mStateExec = 0;
				mCodexExec = pcodex;
				cFlag = true;
			}
			break;
		}
		pcodex = pcodex->next;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::ExeCodex(void)
{
	 // should check if code memory is STILL valid?
	if(!mCodexExec)
	{
		std::cout << "[SIM Error] Cannot get a codex to execute!" ;
		return false;
	}
	bool cExecOK = this->ExecCode(mCodexExec);
	if(mErrorISA)
		std::cout << "[ISA Error] Invalid Instruction Binary!" ;
	if(mErrorRW)
		std::cout << "[R/W Error] Read/Write Error" ;
	return cExecOK;
}
//------------------------------------------------------------------------------
bool my1Sim85::ReadMemory(aword anAddress, abyte& rData)
{
	return mMemory.Read(anAddress,rData);
}
//------------------------------------------------------------------------------
bool my1Sim85::WriteMemory(aword anAddress, abyte aData)
{
	return mMemory.Write(anAddress,aData);
}
//------------------------------------------------------------------------------
bool my1Sim85::ReadDevice(abyte anAddress, abyte& rData)
{
	return mDevice.Read(anAddress,rData);
}
//------------------------------------------------------------------------------
bool my1Sim85::WriteDevice(abyte anAddress, abyte aData)
{
	return mDevice.Write(anAddress,aData);
}
//------------------------------------------------------------------------------
bool my1Sim85::ResetSim(int aStart)
{
	mRegPC.SetData((aword) aStart);
	if(DoUpdate)
		(*DoUpdate)((void*)this);
	return this->GetCodex(mRegPC.GetData());
}
//------------------------------------------------------------------------------
bool my1Sim85::StepSim(void)
{
	if(mHalted)
		return true;
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
bool my1Sim85::InsertMemory(my1Memory* aMemory, int anIndex)
{
	return mMemory.Insert((my1Address*)aMemory,anIndex);
}
//------------------------------------------------------------------------------
bool my1Sim85::InsertDevice(my1Device* aDevice, int anIndex)
{
	return mDevice.Insert((my1Address*)aDevice,anIndex);
}
//------------------------------------------------------------------------------
my1Memory* my1Sim85::RemoveMemory(int anIndex)
{
	return (my1Memory*) mMemory.Remove(anIndex);
}
//------------------------------------------------------------------------------
my1Device* my1Sim85::RemoveDevice(int anIndex)
{
	return (my1Device*) mDevice.Remove(anIndex);
}
//------------------------------------------------------------------------------
my1Memory* my1Sim85::GetMemory(int anIndex)
{
	return (my1Memory*) mMemory.Object(anIndex);
}
//------------------------------------------------------------------------------
my1Device* my1Sim85::GetDevice(int anIndex)
{
	return (my1Device*) mDevice.Object(anIndex);
}
//------------------------------------------------------------------------------
bool my1Sim85::BuildDefault(void)
{
	if(!this->InsertMemory(&mROM)) return false;
	if(!this->InsertMemory(&mRAM)) return false;
	if(!this->InsertDevice(&mPPI)) return false;
	return true;
}
//------------------------------------------------------------------------------
bool my1Sim85::BuildReset(void)
{
	while(mMemory.GetCount())
		mMemory.Remove();
	while(mDevice.GetCount())
		mDevice.Remove();
	// reset pin do_update and do_detect functions as well??
	return true;
}
//------------------------------------------------------------------------------
bool my1Sim85::Assemble(const char* aFileName)
{
	mReady = this->LoadCodex((char*)aFileName);
	if(mReady) mReady = this->ResetSim(mStartAddress);
#ifdef MY1DEBUG
	this->PrintCodexInfo();
#endif
	return mReady;
}
//------------------------------------------------------------------------------
bool my1Sim85::Save2HEX(const char* aFileName)
{
	return false;
}
//------------------------------------------------------------------------------
bool my1Sim85::Simulate(int aStep)
{
	if(!mReady) return false;
	mReady = this->RunSim(aStep);
#ifdef MY1DEBUG
	this->PrintCodexInfo();
#endif
	return mReady;
}
//------------------------------------------------------------------------------
my1Reg85* my1Sim85::GetRegister(int anIndex)
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
int my1Sim85::GetMemoryCount(void)
{
	return mMemory.GetCount();
}
//------------------------------------------------------------------------------
int my1Sim85::GetDeviceCount(void)
{
	return mDevice.GetCount();
}
//------------------------------------------------------------------------------
int my1Sim85::GetStateExec(void)
{
	return mStateExec;
}
//------------------------------------------------------------------------------
int my1Sim85::GetCodexLine(void)
{
	return mCodexExec ? mCodexExec->line : 0;
}
//------------------------------------------------------------------------------
void my1Sim85::PrintCodexInfo(void)
{
	if(mReady)
	{
		std::cout << "[Codex Info] Addr: " <<
			std::setw(4) << std::setfill('0') << std::setbase(16) <<
			mCodexExec->addr << ", ";
		std::cout << "Line: " << std::setbase(10) << mCodexExec->line << ", ";
		std::cout << "Data: ";
		for(int cLoop=0;cLoop<mCodexExec->size;cLoop++)
			std::cout << std::setw(2) << std::setfill('0') << std::hex <<
				(int)mCodexExec->data[cLoop] << ", ";
		std::cout << "[System Info] Program Counter: " <<
			std::setw(4) << std::setfill('0') <<
			std::setbase(16) << mRegPC.GetData() << ", ";
		std::cout << "Reg A:" << std::setw(2) << std::setfill('0') <<
			std::hex << (int)mRegMAIN[I8085_REG_A].GetData() << ", ";
		std::cout << "Reg F:" << std::setw(2) << std::setfill('0') <<
			std::hex << (int)mRegMAIN[I8085_REG_F].GetData() << std::endl;
	}
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
