//------------------------------------------------------------------------------
#ifndef __MY1SIM85HPP__
#define __MY1SIM85HPP__
//------------------------------------------------------------------------------
extern "C"
{
#include "my1i8085.h"
}
//------------------------------------------------------------------------------
#define DATASIZE 8
#define ADDRSIZE 16
#define MAX_MEMSIZE (1<<ADDRSIZE)
#define MAX_DEVSIZE (1<<DATASIZE)
#define MAX_MEMCOUNT 16
#define MAX_DEVCOUNT 32
#define MAX_DEVNAME_CHAR 32
#define I8255_PORTA 0
#define I8255_PORTB 1
#define I8255_PORTC 2
#define I8255_PORTC_U 2
#define I8255_PORTC_L 3
#define I8255_CONTROL 3
#define I8255_COUNT 4
#define I8255_PIN_PA0 (I8255_PORTA*DATASIZE+0)
#define I8255_PIN_PA1 (I8255_PORTA*DATASIZE+1)
#define I8255_PIN_PA2 (I8255_PORTA*DATASIZE+2)
#define I8255_PIN_PA3 (I8255_PORTA*DATASIZE+3)
#define I8255_PIN_PA4 (I8255_PORTA*DATASIZE+4)
#define I8255_PIN_PA5 (I8255_PORTA*DATASIZE+5)
#define I8255_PIN_PA6 (I8255_PORTA*DATASIZE+6)
#define I8255_PIN_PA7 (I8255_PORTA*DATASIZE+7)
#define I8255_PIN_PB0 (I8255_PORTB*DATASIZE+0)
#define I8255_PIN_PB1 (I8255_PORTB*DATASIZE+1)
#define I8255_PIN_PB2 (I8255_PORTB*DATASIZE+2)
#define I8255_PIN_PB3 (I8255_PORTB*DATASIZE+3)
#define I8255_PIN_PB4 (I8255_PORTB*DATASIZE+4)
#define I8255_PIN_PB5 (I8255_PORTB*DATASIZE+5)
#define I8255_PIN_PB6 (I8255_PORTB*DATASIZE+6)
#define I8255_PIN_PB7 (I8255_PORTB*DATASIZE+7)
#define I8085_REG_B 0
#define I8085_REG_C 1
#define I8085_REG_D 2
#define I8085_REG_E 3
#define I8085_REG_H 4
#define I8085_REG_L 5
#define I8085_REG_F 6
#define I8085_REG_M 6
#define I8085_REG_A 7
#define I8085_RP_BC 0
#define I8085_RP_DE 1
#define I8085_RP_HL 2
#define I8085_RP_SP 3
#define I8085_RP_PC 4
#define I8085_FLAG_C 0x01
#define I8085_FLAG_P 0x04
#define I8085_FLAG_A 0x10
#define I8085_FLAG_Z 0x40
#define I8085_FLAG_S 0x80
//------------------------------------------------------------------------------
class my1Memory
{
protected:
	bool mReadOnly, mProgramMode; // program mode allow write to read only!
	char mName[MAX_DEVNAME_CHAR];
	aword mStart, mSize, mLastUsed;
	abyte *mSpace;
public:
	my1Memory(char *aName, int aStart=0x0, int aSize=MAX_MEMSIZE);
	virtual ~my1Memory();
	void (*DoUpdate)(void*); // public function pointer
	bool IsReadOnly(void);
	void ProgramMode(bool aStatus=true);
	const char* GetName(void);
	int GetStart(void);
	int GetSize(void);
	int GetLastUsed(void);
	bool ReadData(aword,abyte&);
	bool WriteData(aword,abyte);
	bool IsSelected(aword);
	bool IsWithin(int,int);
	bool IsWithin(my1Memory&);
};
//------------------------------------------------------------------------------
class my1Sim2764 : public my1Memory
{
public:
	my1Sim2764(int aStart=0x0);
	virtual ~my1Sim2764(){}
};
//------------------------------------------------------------------------------
class my1Sim6264 : public my1Memory
{
public:
	my1Sim6264(int aStart=0x0);
	virtual ~my1Sim6264(){}
};
//------------------------------------------------------------------------------
class my1Device : public my1Memory // acts like a memory?
{
protected:
	bool *mIsInput;
	bool mBuffered;
	bool ReadData(aword,abyte&);
	bool WriteData(aword,abyte);
public:
	my1Device(char *aName, int aStart=0x0, int aSize=MAX_DEVSIZE);
	virtual ~my1Device();
	bool IsInput(int);
	void SetInput(int,bool aStatus=true);
	bool IsBuffered(void);
	void SetBuffered(bool aStatus=true);
	virtual bool ReadDevice(abyte,abyte&);
	virtual bool WriteDevice(abyte,abyte);
	// methods for 'external' device?
	virtual abyte GetData(int);
	virtual void PutData(int,abyte,abyte aMask=0xff);
};
//------------------------------------------------------------------------------
class my1Sim8255 : public my1Device
{
public:
	my1Sim8255(int aStart=0x0);
	virtual ~my1Sim8255(){}
	// override parent methods!
	virtual bool ReadDevice(abyte,abyte&);
	virtual bool WriteDevice(abyte,abyte);
	virtual abyte GetData(int);
	virtual void PutData(int,abyte,abyte aMask=0xff);
};
//------------------------------------------------------------------------------
class my1Sim8085
{
protected:
	bool mHalted, mIEnabled;
	abyte mIntrReg;
	aword mRegPairs[4]; // 4x16-bit registers or 8x8-bit registers
	aword mRegPC, mRegSP;
	int mMemCount, mDevCount, mCodCount, mStateExec;
	my1Memory* mMems[MAX_MEMCOUNT];
	my1Device* mDevs[MAX_DEVCOUNT];
	CODEX *mCodexList, *mCodexExec;
protected:
	void ProgramMode(bool aStatus=true);
	void LoadStuff(STUFFS*);
	bool GetCodex(aword);
	void ExeDelay(void);
	bool ExeCodex(void);
	abyte* GetReg8(abyte);
	aword* GetReg16(abyte,bool aUsePSW=false);
	abyte GetParity(abyte);
	abyte GetSrcData(abyte);
	void PutDstData(abyte,abyte);
	void DoStackPush(aword*);
	void DoStackPop(aword*);
	bool ChkFlag(abyte);
private:
	void ExecMOV(abyte,abyte);
	void ExecMOVi(abyte,abyte);
	void ExecALU(abyte,abyte);
	void ExecALUi(abyte,abyte);
	void ExecDAD(abyte);
	void ExecLXI(abyte,aword);
	void ExecSTAXLDAX(abyte);
	void ExecSTALDA(abyte,aword);
	void ExecSLHLD(abyte,aword);
	void ExecINXDCX(abyte);
	void ExecINRDCR(abyte,abyte);
	void ExecROTATE(abyte);
	void ExecDCSC(abyte);
	void ExecRSIM(abyte);
	void ExecPUSH(abyte);
	void ExecPOP(abyte);
	void ExecCALL(aword);
	void ExecRET(void);
	void ExecRSTn(abyte);
	void ExecJMP(aword);
	void ExecOUTIN(abyte,abyte);
	void ExecCHG(abyte);
	void ExecDIEI(abyte);
	void ExecPCSPHL(abyte);
public:
	my1Sim8085();
	virtual ~my1Sim8085();
	void (*DoDelay)(void*);
	void (*DoUpdate)(void*);
	int GetMemoryCount(void);
	int GetDeviceCount(void);
	int GetStateExec(void);
	int GetCodexLine(void);
	// system setup
	bool InsertMemory(my1Memory*,int anIndex=-1);
	bool InsertDevice(my1Device*,int anIndex=-1);
	my1Memory* RemoveMemory(int);
	my1Device* RemoveDevice(int);
	my1Memory* GetMemory(int);
	my1Device* GetDevice(int);
	// for external access
	int GetRegValue(int,bool aReg16=false);
protected:
	// used by codex
	bool ReadMemory(aword,abyte&);
	bool WriteMemory(aword,abyte);
	bool ReadDevice(abyte,abyte&);
	bool WriteDevice(abyte,abyte);
	// simulation setup
	bool ResetCodex(void);
	bool LoadCodex(char*);
	// simulation functions
	bool ResetSim(int aStart=0x0);
	bool StepSim(void);
	bool RunSim(int aStep=1);
};
//------------------------------------------------------------------------------
class my1Sim85 : public my1Sim8085
{
protected:
	bool mReady;
	int mStartAddress;
public: // for now
	my1Sim2764 mROM;
	my1Sim6264 mRAM;
	my1Sim8255 mPPI;
public:
	my1Sim85(bool aDefaultConfig=false);
	virtual ~my1Sim85();
	void SetStartAddress(int);
	int GetStartAddress(void);
	bool Assemble(const char*);
	bool Simulate(int aStep=1);
	void PrintCodexInfo(void);
};
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
