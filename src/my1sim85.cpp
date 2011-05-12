//------------------------------------------------------------------------------
#include "my1sim85.hpp"
//------------------------------------------------------------------------------
static const char I8255_NAME[]="8255";
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Memory::my1Memory(int aStart, int aSize)
{
	mStart = aStart;
	mSize = aSize;
	mSpace = new abyte[mSize];
}
//------------------------------------------------------------------------------
my1Memory::~my1Memory()
{
	delete mSpace;
}
//------------------------------------------------------------------------------
int my1Memory::GetStart(void)
{
	return mStart;
}
//------------------------------------------------------------------------------
int my1Memory::GetSize(void)
{
	return mSize;
}
//------------------------------------------------------------------------------
bool my1Memory::ReadData(aword anAddress, abyte& rData)
{
	if(anAddress<mStart||anAddress>=mStart+mSize)
		return false;
	rData = mSpace[anAddress-mStart];
	return true;
}
//------------------------------------------------------------------------------
bool my1Memory::WriteData(aword anAddress, abyte aData)
{
	if(anAddress<mStart||anAddress>=mStart+mSize)
		return false;
	mSpace[anAddress-mStart] = aData;
	return true;
}
//------------------------------------------------------------------------------
bool my1Memory::IsSelected(aword anAddress)
{
	if(anAddress<mStart||anAddress>=mStart+mSize)
		return false;
	return true;
}
//------------------------------------------------------------------------------
bool my1Memory::IsWithin(int aStart, int aSize)
{
	bool cFlag = false;
	if((aStart>=mStart&&aStart<mStart+mSize)||
		(mStart>=aStart&&mStart<aStart+aSize))
		cFlag = true;
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Memory::IsWithin(my1Memory& rMemory)
{
	return this->IsWithin(rMemory.mStart,rMemory.mSize);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Device::my1Device(char *aName, int aStart, int aSize)
	: my1Memory(aStart, aSize)
{
	int cCount = 0;
	if(aName)
	{
		while(aName[cCount]&&cCount<MAX_DEVNAME_CHAR-1)
		{
			mName[cCount] = aName[cCount];
			cCount++;
		}
	}
	mName[cCount] = 0x0;
}
//------------------------------------------------------------------------------
const char* my1Device::GetName(void)
{
	return (const char*) mName;
}
//------------------------------------------------------------------------------
bool my1Device::ReadDevice(abyte anAddress, abyte& rData)
{
	return ReadData(anAddress, rData);
}
//------------------------------------------------------------------------------
bool my1Device::WriteDevice(abyte anAddress, abyte aData)
{
	return WriteData(anAddress, aData);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
my1Sim8255::my1Sim8255(int aStart)
	: my1Device((char*)I8255_NAME, aStart, 4)
{
	// by default config is random?
}
//------------------------------------------------------------------------------
bool my1Sim8255::ReadDevice(abyte anAddress, abyte& rData)
{
	bool cFlag = true;
	int cIndex = anAddress-mStart;
	switch(cIndex)
	{
		case I8255_PORTA:
		case I8255_PORTB:
			if(mIsInput[cIndex]) rData = mSpace[cIndex];
			break;
		case I8255_PORTC:
			if(mIsInput[I8255_PORTC_U])
				 rData = (mSpace[cIndex] & 0xF0) | (rData & 0x0F);
			if(!mIsInput[I8255_PORTC_L])
				 rData = (mSpace[cIndex] & 0x0F) | (rData & 0xF0);
			break;
		case I8255_CONTROL:
		default:
			cFlag = false;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim8255::WriteDevice(abyte anAddress, abyte aData)
{
	bool cFlag = true;
	int cIndex = anAddress-mStart;
	switch(cIndex)
	{
		case I8255_PORTA:
		case I8255_PORTB:
			if(!mIsInput[cIndex]) mSpace[cIndex] = aData;
			break;
		case I8255_PORTC:
			if(!mIsInput[I8255_PORTC_U])
				mSpace[cIndex] = (aData & 0xF0) | (mSpace[cIndex] & 0x0F);
			if(!mIsInput[I8255_PORTC_L])
				mSpace[cIndex] = (aData & 0x0F) | (mSpace[cIndex] & 0xF0);
			break;
		case I8255_CONTROL:
			if(aData&0x80)
			{
				// config data - always assume mode 0!
				if(aData&0x10) mIsInput[I8255_PORTA] = true;
				else mIsInput[I8255_PORTA] = false;
				if(aData&0x02) mIsInput[I8255_PORTB] = true;
				else mIsInput[I8255_PORTB] = false;
				if(aData&0x08) mIsInput[I8255_PORTC_U] = true;
				else mIsInput[I8255_PORTC_U] = false;
				if(aData&0x01) mIsInput[I8255_PORTC_L] = true;
				else mIsInput[I8255_PORTC_L] = false;
			}
			else
			{
				// bsr mode!
				abyte cCount = aData&0x0e;
				abyte cData = aData&0x01;
				cData <<= cCount;
				if((cCount<4&&!mIsInput[I8255_PORTC_L])||(cCount>3&&!mIsInput[I8255_PORTC_U]))
				{
					if(cData)
						mSpace[I8255_PORTC] |= cData;
					else
						mSpace[I8255_PORTC] &= ~cData;
				}
			}
			break;
		default:
			cFlag = false;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
abyte my1Sim8255::GetData(int anIndex)
{
	if(mIsInput[anIndex%4])
		return 0x00;
	return mSpace[anIndex%4];
}
//------------------------------------------------------------------------------
void my1Sim8255::PutData(int anIndex, abyte aData, abyte aMask)
{
	if(!mIsInput[anIndex%4])
		return;
	mSpace[anIndex%4] = (aData & aMask) | (mSpace[anIndex%4] & ~aMask);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void my1Sim85::LoadStuff(STUFFS* pstuffs)
{
	// following c-style coding (originate from my1i8085!)
	CODEX *pcodex, *tcodex, *ccodex= mCodexList; // should already be 0x0!
	// check if there are any codes?
	pcodex = pstuffs->pcodex;
	if(!pcodex)
		return;
	pstuffs->addr = pcodex->addr;
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
		mCodCount++;
		/* check for error/discontinuity */
		if(pcodex->addr<pstuffs->addr)
			pstuffs->errc++;
		else if(pcodex->addr>pstuffs->addr)
			pstuffs->addr = pcodex->addr;
		/* fill memory with codex data */
		for(int cLoop=0;cLoop<pcodex->size;cLoop++)
		{
			if(!this->WriteMemory(pstuffs->addr++,pcodex->data[cLoop]))
				pstuffs->errc++;
		}
		pcodex = pcodex->next;
	}
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
void my1Sim85::ExeDelay(void)
{
	if(DoDelay&&mStateExec)
		(*DoDelay)((void*)this);
}
//------------------------------------------------------------------------------
bool my1Sim85::ExeCodex(void)
{
	CODEX *pcodex = mCodexExec; // just to make the code look consistent! ;p
	 // should check if code memory is STILL valid?
	if(!pcodex)
		return false;
	// update program counter? do this here!
	mRegPC += pcodex->size;
	// check opcode!
	if((pcodex->data[0]&0xC0)==0x40) // MOV group
	{
		if((pcodex->data[0]&0x3F)==0x36) // check for HALT!
		{
			mHalted = true;
			mStateExec = 5;
			this->ExeDelay();
		}
		else
		{
			mStateExec = 4;
			if((pcodex->data[0]&0x38)==0x30||(pcodex->data[0]&0x07)==0x06)
				mStateExec += 3;
			this->ExeDelay();
			this->ExecMOV((pcodex->data[0]&0x38)>>3,(pcodex->data[0]&0x07));
		}
	}
	else if((pcodex->data[0]&0xC0)==0x80) // ALU group
	{
		mStateExec = 4;
		if((pcodex->data[0]&0x07)==0x06)
			mStateExec+=3;
		this->ExeDelay();
		this->ExecALU((pcodex->data[0]&0x38)>>3,(pcodex->data[0]&0x07));
	}
	else if((pcodex->data[0]&0xC0)==0x00) // data proc group
	{
		if((pcodex->data[0]&0x07)==0x06) // mvi
		{
			mStateExec = 7;
			this->ExeDelay();
			this->ExecMOVi((pcodex->data[0]&0x38)>>3,pcodex->data[1]);
		}
		else if((pcodex->data[0]&0x0F)==0x01) // lxi
		{
			aword *pdata = (aword*) &pcodex->data[1];
			mStateExec = 10;
			this->ExeDelay();
			this->ExecLXI((pcodex->data[0]&0x30)>>4,*pdata);
		}
		else if((pcodex->data[0]&0x0F)==0x09) // dad
		{
			mStateExec = 10;
			this->ExeDelay();
			this->ExecDAD((pcodex->data[0]&0x30)>>4);
		}
		else if((pcodex->data[0]&0x27)==0x02) // stax/ldax
		{
			mStateExec = 7;
			this->ExeDelay();
			this->ExecSTAXLDAX((pcodex->data[0]&0x18)>>3);
		}
		else if((pcodex->data[0]&0x37)==0x22) // shld/lhld
		{
			aword *pdata = (aword*) &pcodex->data[1];
			mStateExec = 16;
			this->ExeDelay();
			this->ExecSLHLD((pcodex->data[0]&0x08)>>3,*pdata);
		}
		else if((pcodex->data[0]&0x37)==0x32) // sta/lda
		{
			aword *pdata = (aword*) &pcodex->data[1];
			mStateExec = 13;
			this->ExeDelay();
			this->ExecSTALDA((pcodex->data[0]&0x08)>>3,*pdata);
		}
		else if((pcodex->data[0]&0x07)==0x03) // inx/dcx
		{
			mStateExec = 6;
			this->ExeDelay();
			this->ExecINXDCX((pcodex->data[0]&0x38)>>3);
		}
		else if((pcodex->data[0]&0x06)==0x04) // inr/dcr
		{
			mStateExec = 4;
			if((pcodex->data[0]&0x38)==0x30)
				mStateExec+=6;
			this->ExeDelay();
			this->ExecINRDCR((pcodex->data[0]&0x01),(pcodex->data[0]&0x38)>>3);
		}
		else if((pcodex->data[0]&0x27)==0x07) // rotates
		{
			mStateExec = 4;
			this->ExeDelay();
			this->ExecROTATE((pcodex->data[0]&0x18)>>3);
		}
		else if((pcodex->data[0]&0x27)==0x17) // misc - daa, cma, stc, cmc
		{
			mStateExec = 4;
			this->ExeDelay();
			this->ExecDCSC((pcodex->data[0]&0x18)>>3);
		}
		else if((pcodex->data[0]&0x2F)==0x20) // rim, sim
		{
			mStateExec = 4;
			this->ExeDelay();
			this->ExecRSIM((pcodex->data[0]&0x10)>>4);
		}
		else if(pcodex->data[0]==0x00) // nop
		{
			mStateExec = 4;
			this->ExeDelay();
			// NOP!
		}
		else // unspecified instructions (0x08, 0x10, 0x18, 0x28, 0x38)
		{
			return false;
		}
	}
	else if((pcodex->data[0]&0xC0)==0xC0) // control group
	{
		if((pcodex->data[0]&0x07)==0x06) // alu_i group
		{
			mStateExec = 7;
			this->ExeDelay();
			this->ExecALUi((pcodex->data[0]&0x38)>>3,pcodex->data[1]);
		}
		else if((pcodex->data[0]&0x0b)==0x01) // push/pop
		{
			mStateExec = 10;
			if((pcodex->data[0]&0x04))
				mStateExec += 2;
			this->ExeDelay();
			if((pcodex->data[0]&0x04))
				this->ExecPUSH((pcodex->data[0]&0x30)>>4);
			else
				this->ExecPOP((pcodex->data[0]&0x30)>>4);
		}
		else if((pcodex->data[0]&0x3b)==0x09) // call/ret
		{
			aword *pdata = (aword*) &pcodex->data[1];
			mStateExec = 10;
			if((pcodex->data[0]&0x04))
				mStateExec += 8;
			this->ExeDelay();
			if((pcodex->data[0]&0x04))
				this->ExecCALL(*pdata);
			else
				this->ExecRET();
		}
		else if((pcodex->data[0]&0x07)==0x04) // call conditional
		{
			aword *pdata = (aword*) &pcodex->data[1];
			bool cDoThis = this->ChkFlag((pcodex->data[0]&0x38)>>3);
			mStateExec = 9;
			if(cDoThis)
				mStateExec += 9;
			this->ExeDelay();
			if(cDoThis)
				this->ExecCALL(*pdata);
		}
		else if((pcodex->data[0]&0x07)==0x00) // return conditional
		{
			bool cDoThis = this->ChkFlag((pcodex->data[0]&0x38)>>3);
			mStateExec = 6;
			if(cDoThis)
				mStateExec += 6;
			this->ExeDelay();
			if(cDoThis)
				this->ExecRET();
		}
		else if((pcodex->data[0]&0x07)==0x07) // rst n
		{
			mStateExec = 12;
			this->ExeDelay();
			this->ExecRSTn((pcodex->data[0]&0x38)>>3);
		}
		else if((pcodex->data[0]&0x3F)==0x03) // jmp
		{
			aword *pdata = (aword*) &pcodex->data[1];
			mStateExec = 10;
			this->ExeDelay();
			this->ExecJMP(*pdata);
		}
		else if((pcodex->data[0]&0x07)==0x02) // jump conditional
		{
			aword *pdata = (aword*) &pcodex->data[1];
			bool cDoThis = this->ChkFlag((pcodex->data[0]&0x38)>>3);
			mStateExec = 7;
			if(cDoThis)
				mStateExec += 3;
			this->ExeDelay();
			if(cDoThis)
				this->ExecJMP(*pdata);
		}
		else if((pcodex->data[0]&0x37)==0x13) // out/in
		{
			mStateExec = 10;
			this->ExeDelay();
			this->ExecOUTIN((pcodex->data[0]&0x08)>>3, pcodex->data[1]);
		}
		else if((pcodex->data[0]&0x37)==0x23) // xthl/xchg
		{
			mStateExec = 4;
			if(!(pcodex->data[0]&0x08))
				mStateExec += 16;
			this->ExeDelay();
			this->ExecCHG((pcodex->data[0]&0x08)>>3);
		}
		else if((pcodex->data[0]&0x37)==0x33) // di/ei
		{
			mStateExec = 4;
			this->ExeDelay();
			this->ExecDIEI((pcodex->data[0]&0x08)>>3);
		}
		else if((pcodex->data[0]&0x29)==0x29) // pchl/sphl
		{
			mStateExec = 4;
			this->ExeDelay();
			this->ExecPCSPHL((pcodex->data[0]&0x10)>>4);
		}
		else // unspecified instructions (0xcb, 0xd9, 0xdd, 0xed, 0xfd)
		{
			return false;
		}
	}
	return true;
}
//------------------------------------------------------------------------------
abyte* my1Sim85::GetReg8(abyte anIndex)
{
	abyte* pReg8 = (abyte*) mRegPairs;
	return (abyte*) &pReg8[(anIndex^0x01)];
}
//------------------------------------------------------------------------------
aword* my1Sim85::GetReg16(abyte anIndex, bool aUsePSW)
{
	aword *pReg16 = 0x00;
	switch(anIndex)
	{
		case I8085_RP_BC:
		case I8085_RP_DE:
		case I8085_RP_HL:
			pReg16 = &mRegPairs[anIndex];
			break;
		case I8085_RP_SP:
			if(aUsePSW) pReg16 = &mRegPairs[anIndex];
			else pReg16 = &mRegSP;
			break;
	}
	return pReg16;
}
//------------------------------------------------------------------------------
abyte my1Sim85::GetParity(abyte aData)
{
	abyte cOddParity = 0x01;
	for(int cLoop=0;cLoop<8;cLoop++)
	{
		cOddParity ^= (aData&0x01);
		aData >>= 1;
	}
	return (cOddParity << 6);
}
//------------------------------------------------------------------------------
abyte my1Sim85::GetSrcData(abyte src)
{
	abyte cData;
	switch(src)
	{
		case I8085_REG_M:
			this->ReadMemory(mRegPairs[I8085_RP_HL],cData);
 			break;
		default:
			cData = *GetReg8(src);
	}
	return cData;
}
//------------------------------------------------------------------------------
void my1Sim85::PutDstData(abyte dst, abyte aData)
{
	switch(dst)
	{
		case I8085_REG_M:
			this->WriteMemory(mRegPairs[I8085_RP_HL],aData);
 			break;
		default:
			*GetReg8(dst) = aData;
	}
}
//------------------------------------------------------------------------------
void my1Sim85::DoStackPush(aword* pData)
{
	abyte *pDataL = (abyte*) pData;
	abyte *pDataH = (abyte*) (pData+1);
	this->WriteMemory(--mRegSP,*pDataH);
	this->WriteMemory(--mRegSP,*pDataL);
}
//------------------------------------------------------------------------------
void my1Sim85::DoStackPop(aword* pData)
{
	abyte *pDataL = (abyte*) pData;
	abyte *pDataH = (abyte*) (pData+1);
	this->ReadMemory(mRegSP++,*pDataL);
	this->ReadMemory(mRegSP++,*pDataH);
}
//------------------------------------------------------------------------------
bool my1Sim85::ChkFlag(abyte sel)
{
	bool cStatus = false;
	abyte cFlag = *GetReg8(I8085_REG_F);
	switch(sel&0x06)
	{
		case 0: // carry
			cStatus = (cFlag&I8085_FLAG_C) ? true : false;
			if(sel&0x01)
				cStatus = !cStatus;
		case 2: // sign
			cStatus = (cFlag&I8085_FLAG_S) ? false : true;
			if(sel&0x01)
				cStatus = !cStatus;
		case 4: // zero
			cStatus = (cFlag&I8085_FLAG_Z) ? true : false;
			if(sel&0x01)
				cStatus = !cStatus;
		case 8: // parity
			cStatus = (cFlag&I8085_FLAG_P) ? true : false;
			if(sel&0x01)
				cStatus = !cStatus;
	}
	return cStatus;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecMOV(abyte dst, abyte src)
{
	this->PutDstData(dst,this->GetSrcData(src));
}
//------------------------------------------------------------------------------
void my1Sim85::ExecMOVi(abyte dst, abyte aData)
{
	this->PutDstData(dst,aData);
}
//------------------------------------------------------------------------------
void my1Sim85::ExecALU(abyte sel, abyte src)
{
	this->ExecALUi(sel,GetSrcData(src));
}
//------------------------------------------------------------------------------
void my1Sim85::ExecALUi(abyte sel, abyte aData)
{
	abyte cFlag, *pReg8 = GetReg8(I8085_REG_A);
	// do the operation!
	if((sel&0x04)&&(sel!=0x07)) // logical excluding cmp!
	{
		cFlag = 0x00; // by default ac reset, cy reset!
		switch(sel&0x03)
		{
			case 0x00:
				aData &= *pReg8;
				cFlag = 0x10; // ac set, cy reset!
				break;
			case 0x01:
				aData ^= *pReg8;
				break;
			case 0x10:
				aData |= *pReg8;
				break;
		}
		*pReg8 = aData;
	}
	else // arithmetic
	{
		aword cTestX, cTestY;
		if(sel==0x07) // actually a compare, do subtract
			sel = 0x06;
		cTestX = *pReg8;
		cTestY = (*pReg8)&0x0F;
		if(sel&0x02)
		{
			cTestX -= (aword)aData+(sel&0x01);
			cTestY -= (aword)(aData&0x0F)+(sel&0x01);
		}
		else
		{
			cTestX += (aword)aData+(sel&0x01);
			cTestY += (aword)(aData&0x0F)+(sel&0x01);
		}
		aData = (cTestX&0xFF);
		if(!(sel&0x04)) // cmp doesn't update accumulator!
			(*pReg8) = aData;
		// update flag!
		cFlag = 0x00;
		if(cTestX&0x100) // carry
			cFlag |= 0x01;
		if(cTestY&0x10) // aux carry
			cFlag |= 0x10;
	}
	// update flag!
	if(aData==0x00) cFlag |= 0x40; // zero
	cFlag |= (aData&0x80); // sign flag
	cFlag |= GetParity(aData); // parity flag
	pReg8 = GetReg8(I8085_REG_F);
	*pReg8 = cFlag;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecDAD(abyte sel)
{
	mRegPairs[I8085_RP_HL] += *GetReg16(sel);
}
//------------------------------------------------------------------------------
void my1Sim85::ExecLXI(abyte sel, aword aData)
{
	*GetReg16(sel) = aData;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecSTAXLDAX(abyte sel)
{
	this->ExecSTALDA(sel, mRegPairs[(sel&0x02)>>1]);
}
//------------------------------------------------------------------------------
void my1Sim85::ExecSTALDA(abyte sel, aword anAddr)
{
	abyte cData;
	abyte *pReg8 = GetReg8(I8085_REG_A);
	// do the transfer!
	if(sel&0x01)
	{
		this->ReadMemory(anAddr,cData);
		*pReg8 = cData;
	}
	else
	{
		cData = *pReg8;
		this->WriteMemory(anAddr,cData);
	}
}
//------------------------------------------------------------------------------
void my1Sim85::ExecSLHLD(abyte sel, aword anAddr)
{
	abyte cData;
	// do the transfer!
	if(sel&0x01)
	{
		this->ReadMemory(anAddr++,cData);
		*GetReg8(I8085_REG_L) = cData;
		this->ReadMemory(anAddr,cData);
		*GetReg8(I8085_REG_H) = cData;
	}
	else
	{
		cData = *GetReg8(I8085_REG_L);
		this->WriteMemory(anAddr++,cData);
		cData = *GetReg8(I8085_REG_H);
		this->WriteMemory(anAddr,cData);
	}
}
//------------------------------------------------------------------------------
void my1Sim85::ExecINXDCX(abyte sel)
{
	if(sel&0x01)
		(*GetReg16((sel&0X06)>>1))--;
	else
		(*GetReg16((sel&0X06)>>1))++;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecINRDCR(abyte sel, abyte reg)
{
	abyte cData, cFlag;
	aword cTestX = this->GetSrcData(reg);
	aword cTestY = cTestX&0x000F;
	if(sel&0x01) { cTestX--; cTestY--; }
	else { cTestX++; cTestY++; }
	// get results
	cData = cTestX&0xFF;
	cFlag = 0x00;
	if(cTestX&0x100) // carry
		cFlag |= 0x01;
	if(cTestY&0x10) // aux carry
		cFlag |= 0x10;
	if(cData==0x00) cFlag |= 0x40; // zero
	cFlag |= (cData&0x80); // sign flag
	cFlag |= GetParity(cData); // parity flag
	// write-back!
	*GetReg8(I8085_REG_F) = cFlag;
	this->PutDstData(reg,cData);
}
//------------------------------------------------------------------------------
void my1Sim85::ExecROTATE(abyte sel)
{
	abyte cTempC, cTempF;
	abyte *pReg8A = GetReg8(I8085_REG_A);
	abyte *pReg8F = GetReg8(I8085_REG_F);
	if(sel&0x01) // rotate right
	{
		cTempC = (*pReg8A)&0x01;
		if(sel&0x02) cTempF = (*pReg8F)&0x01;
		else cTempF = cTempC;
		(*pReg8A) = ((*pReg8A)>>1) | (cTempF<<7);
	}
	else // rotate left
	{
		cTempC = ((*pReg8A)&0x80)>>7;
		if(sel&0x02) cTempF = (*pReg8F)&0x01;
		else cTempF = cTempC;
		(*pReg8A) = ((*pReg8A)<<1) | (cTempF);
	}
	// update carry flag
	if(cTempC)
		(*pReg8F) |= cTempC;
	else
		(*pReg8F) &= ~cTempC;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecDCSC(abyte sel)
{
	abyte *pReg8A = GetReg8(I8085_REG_A);
	abyte *pReg8F = GetReg8(I8085_REG_F);
	if(sel&0x02) // carry flag ops
	{
		if(sel&0x01) (*pReg8F) ^= 0x01; // complement carry flag
		else (*pReg8F) |= 0x01; // set carry flag
	}
	else // accumulator ops
	{
		if(sel&0x01) (*pReg8A) = ~(*pReg8A); // complement accumulator
		else
		{
			// DAA operation
			abyte cData, cFlag = 0x00;
			aword cTestX, cTestY;
			cTestX = (*pReg8A)&0x00F0;
			cTestY = (*pReg8A)&0x000F;
			// check lower nibble
			if(cTestY>0x09||((*pReg8F)&0x10))
			{
				cTestY += 0x06;
				if(cTestY&0x0010) cFlag |= 0x10;
			}
			// check upper nibble
			cTestX += cFlag;
			if(cTestX>0x90||((*pReg8F)&0x01))
			{
				cTestX += 0x60;
				if(cTestX&0x100) cFlag |= 0x01;
			}
			cData = (cTestX+cTestY)&0xFF;
			// update flag!
			if(cData==0x00) cFlag |= 0x40; // zero
			cFlag |= (cData&0x80); // sign flag
			cFlag |= GetParity(cData); // parity flag
			*pReg8F = cFlag;
			*pReg8A = cData;
		}
	}
}
//------------------------------------------------------------------------------
void my1Sim85::ExecRSIM(abyte sel)
{
	// ONLY A 'PLACEHOLDER' NOT PROPERLY IMPLEMENTED!
	if(sel)
	{
		// sim
		mIntrReg = *GetReg8(I8085_REG_A);
	}
	else
	{
		// rim
		*GetReg8(I8085_REG_A) = mIntrReg;
	}
}
//------------------------------------------------------------------------------
void my1Sim85::ExecPUSH(abyte sel)
{
	this->DoStackPush(GetReg16(sel,true));
}
//------------------------------------------------------------------------------
void my1Sim85::ExecPOP(abyte sel)
{
	this->DoStackPop(GetReg16(sel,true));
}
//------------------------------------------------------------------------------
void my1Sim85::ExecCALL(aword anAddress)
{
	this->DoStackPush(&mRegPC);
	mRegPC = anAddress;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecRET(void)
{
	this->DoStackPop(&mRegPC);
}
//------------------------------------------------------------------------------
void my1Sim85::ExecRSTn(abyte sel)
{
	aword cTarget = sel * 8;
	this->ExecCALL(cTarget);
}
//------------------------------------------------------------------------------
void my1Sim85::ExecJMP(aword anAddress)
{
	mRegPC = anAddress;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecOUTIN(abyte sel, abyte anAddress)
{
	if(sel&0x01)
		this->ReadDevice(anAddress,*GetReg8(I8085_REG_A));
	else
		this->WriteDevice(anAddress,*GetReg8(I8085_REG_A));
}
//------------------------------------------------------------------------------
void my1Sim85::ExecCHG(abyte sel)
{
	aword cTemp;
	if(sel&0x01) // xchg
	{
		// swap DE and HL
		cTemp = mRegPairs[I8085_RP_DE];
		mRegPairs[I8085_RP_DE] = mRegPairs[I8085_RP_HL];
		mRegPairs[I8085_RP_HL] = cTemp;
	}
	else
	{
		this->DoStackPop(&cTemp);
		this->DoStackPush(&mRegPairs[I8085_RP_HL]);
		mRegPairs[I8085_RP_HL] = cTemp;
	}
}
//------------------------------------------------------------------------------
void my1Sim85::ExecDIEI(abyte sel)
{
	mIEnabled = (sel&0x01) ? true : false;
}
//------------------------------------------------------------------------------
void my1Sim85::ExecPCSPHL(abyte sel)
{
	if(sel&0x01) // hl to sp
		mRegSP = mRegPairs[I8085_RP_HL];
	else // hl to pc
		mRegPC = mRegPairs[I8085_RP_HL];
}
//------------------------------------------------------------------------------
my1Sim85::my1Sim85()
{
	mHalted = false;
	mIEnabled = false;
	mIntrReg = 0x00; // not sure!
	// mRegPairs[] random?!
	mRegPC = 0x0000;
	// mSPointer random?!
	mMemCount = 0;
	for(int cLoop=0;cLoop<MAX_MEMCOUNT;cLoop++)
		mMems[cLoop] = 0x0;
	mDevCount = 0;
	for(int cLoop=0;cLoop<MAX_DEVCOUNT;cLoop++)
		mDevs[cLoop] = 0x0;
	mCodCount = 0;
	mCodexList = 0x0;
	mStateExec = 0;
	mCodexExec = 0x0;
	DoDelay = 0x0;
}
//------------------------------------------------------------------------------
my1Sim85::~my1Sim85()
{
	this->ResetCodex();
}
//------------------------------------------------------------------------------
int my1Sim85::GetStateExec(void)
{
	return mStateExec;
}
//------------------------------------------------------------------------------
int my1Sim85::GetCodexLine(void)
{
	int cLine = 0;
	if(mCodexExec)
		cLine = mCodexExec->line;
	return cLine;
}
//------------------------------------------------------------------------------
bool my1Sim85::AddMemory(my1Memory* aMemory)
{
	// check existing memory space
	for(int cLoop=0;cLoop<mMemCount;cLoop++)
	{
		if(aMemory->IsWithin(*mMems[cLoop]))
			return false;
	}
	// okay... insert!
	mMems[mMemCount++] = aMemory;
	return true;
}
//------------------------------------------------------------------------------
bool my1Sim85::AddDevice(my1Device* aDevice)
{
	// check existing i/o space
	for(int cLoop=0;cLoop<mDevCount;cLoop++)
	{
		if(aDevice->IsWithin(*mDevs[cLoop]))
			return false;
	}
	// okay... insert!
	mDevs[mDevCount++] = aDevice;
	return true;
}
//------------------------------------------------------------------------------
my1Memory* my1Sim85::GetMemory(int aStart)
{
	my1Memory* cTemp = 0x0;
	for(int cLoop=0;cLoop<mMemCount;cLoop++)
	{
		if(aStart==mMems[cLoop]->GetStart())
		{
			cTemp = mMems[cLoop];
			break;
		}
	}
	return cTemp;
}
//------------------------------------------------------------------------------
my1Device* my1Sim85::GetDevice(int aStart)
{
	my1Device* cTemp = 0x0;
	for(int cLoop=0;cLoop<mDevCount;cLoop++)
	{
		if(aStart==mDevs[cLoop]->GetStart())
		{
			cTemp = mDevs[cLoop];
			break;
		}
	}
	return cTemp;
}
//------------------------------------------------------------------------------
bool my1Sim85::ReadMemory(aword anAddress, abyte& rData)
{
	bool cFlag = false;
	for(int cLoop=0;cLoop<mMemCount&&!cFlag;cLoop++)
	{
		cFlag =mMems[cLoop]->ReadData(anAddress, rData);
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::WriteMemory(aword anAddress, abyte aData)
{
	bool cFlag = false;
	for(int cLoop=0;cLoop<mMemCount&&!cFlag;cLoop++)
	{
		cFlag = mMems[cLoop]->WriteData(anAddress, aData);
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::ReadDevice(abyte anAddress, abyte& rData)
{
	bool cFlag = false;
	for(int cLoop=0;cLoop<mDevCount&&!cFlag;cLoop++)
	{
		cFlag = mDevs[cLoop]->ReadDevice(anAddress, rData);
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::WriteDevice(abyte anAddress, abyte aData)
{
	bool cFlag = false;
	for(int cLoop=0;cLoop<mDevCount&&!cFlag;cLoop++)
	{
		cFlag = mDevs[cLoop]->WriteDevice(anAddress, aData);
	}
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::ResetCodex(void)
{
	bool cFlag = true;
	while(mCodexList)
	{
		CODEX* pCodex = mCodexList;
		mCodexList = mCodexList->next;
		free_codex(pCodex);
		mCodCount--;
	}
	if(mCodCount)
	{
		mCodCount = 0;
		cFlag = false;
	}
	mStateExec = 0;
	mCodexExec = 0x0;
	return cFlag;
}
//------------------------------------------------------------------------------
bool my1Sim85::LoadCodex(char *aFilename)
{
	// initialize main data structure
	STUFFS things;
	initialize(&things);
	things.afile = aFilename;
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
		// copy codex to local
		this->ResetCodex();
		this->LoadStuff(&things);
	}
	while(0);
	// clean-up main data structure
	cleanup(&things);
	return things.errc; // still maintained in structure
}
//------------------------------------------------------------------------------
bool my1Sim85::ResetSim(void)
{
	mRegPC = 0x0000;
	return this->GetCodex(mRegPC);
}
//------------------------------------------------------------------------------
bool my1Sim85::StepSim(void)
{
	if(mHalted)
		return true;
	if(!this->ExeCodex())
		return false;
	return this->GetCodex(mRegPC);
}
//------------------------------------------------------------------------------
bool my1Sim85::RunSim(int aStep)
{
	bool cFlag = true;
	while(cFlag&&aStep>0)
	{
		cFlag = this->StepSim();
		aStep--;
	}
	return cFlag;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
