//------------------------------------------------------------------------------
#ifndef __MY1SIM85HPP__
#define __MY1SIM85HPP__
//------------------------------------------------------------------------------
extern "C"
{
#include "my1i8085.h"
}
//------------------------------------------------------------------------------
#define BIT_STATE_0 0x0
#define BIT_STATE_1 0x1
#define BIT_STATE_Z 0x2
//------------------------------------------------------------------------------
#define MAX_SIMNAME_SIZE 32
//------------------------------------------------------------------------------
#define DATASIZE 8
#define ADDRSIZE 16
#define MAX_MEMSIZE (1<<ADDRSIZE)
#define MAX_DEVSIZE (1<<DATASIZE)
#define MAX_MAPSIZE MAX_MEMSIZE
#define MAX_ADDRMAP_COUNT 32
#define MAX_PORTPIN_COUNT 8
#define I2764_NAME "2764"
#define I2764_SIZE 0x2000
#define I6264_NAME "6264"
#define I6264_SIZE 0x2000
#define I8255_NAME "8255"
#define I8255_SIZE 4
#define I8255_DATASIZE 8
#define I8255_PORTA 0
#define I8255_PORTB 1
#define I8255_PORTC 2
#define I8255_CNTRL 3
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
#define I8255_PIN_PC0 (I8255_PORTA*DATASIZE+0)
#define I8255_PIN_PC1 (I8255_PORTA*DATASIZE+1)
#define I8255_PIN_PC2 (I8255_PORTA*DATASIZE+2)
#define I8255_PIN_PC3 (I8255_PORTA*DATASIZE+3)
#define I8255_PIN_PC4 (I8255_PORTA*DATASIZE+4)
#define I8255_PIN_PC5 (I8255_PORTA*DATASIZE+5)
#define I8255_PIN_PC6 (I8255_PORTA*DATASIZE+6)
#define I8255_PIN_PC7 (I8255_PORTA*DATASIZE+7)
#define I8085_REG_COUNT 8
#define I8085_REG_B 0
#define I8085_REG_C 1
#define I8085_REG_D 2
#define I8085_REG_E 3
#define I8085_REG_H 4
#define I8085_REG_L 5
#define I8085_REG_F 6
#define I8085_REG_M 6
#define I8085_REG_A 7
#define I8085_RP_COUNT 4
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
#define I8085_PIN_COUNT 8
#define I8085_PIN_TRAP 0x00
#define I8085_PIN_I7P5 0x01
#define I8085_PIN_I6P5 0x02
#define I8085_PIN_I5P5 0x03
#define I8085_PIN_INTR 0x04
#define I8085_PIN_SID 0x05
#define I8085_PIN_SOD 0x06
#define I8085_PIN_INTA 0x07
#define I8085_ISR_TRP 0x0024
#define I8085_ISR_5P5 0x002C
#define I8085_ISR_6P5 0x0034
#define I8085_ISR_7P5 0x003C
#define I8085_RIM_5P5 0x0
#define I8085_RIM_6P5 0x1
#define I8085_RIM_7P5 0x2
#define I8085_RIM_ENB 0x3
#define I8085_RIM_IS5 0x4
#define I8085_RIM_IS6 0x5
#define I8085_RIM_IS7 0x6
#define I8085_RIM_SID 0x7
#define I8085_SIM_6P5 0x1
#define I8085_SIM_7P5 0x2
#define I8085_SIM_ENB 0x3
#define I8085_SIM_IS5 0x4
#define I8085_SIM_IS6 0x5
#define I8085_SIM_IS7 0x6
#define I8085_SIM_SOD 0x7
//------------------------------------------------------------------------------
class my1SimObject
{
protected:
	char mName[MAX_SIMNAME_SIZE];
	void *mLink;
public:
	void (*DoUpdate)(void*);
	void (*DoDetect)(void*);
	void (*DoDelay)(void*,int);
public:
	my1SimObject();
	virtual ~my1SimObject(){}
	const char* GetName(void);
	void SetName(const char*);
	void* GetLink(void);
	void SetLink(void*);
};
//------------------------------------------------------------------------------
class my1Address : public my1SimObject
{
protected:
	aword mStart, mSize;
public:
	my1Address(int aStart=0x0, int aSize=MAX_MEMSIZE);
	virtual ~my1Address(){}
	int GetStart(void);
	int GetSize(void);
	bool IsOverlapped(int,int);
	bool IsOverlapped(my1Address&);
	virtual bool IsSelected(aword);
	// pure-virtual functions!
	virtual bool ReadData(aword,abyte&) = 0;
	virtual bool WriteData(aword,abyte) = 0;
};
//------------------------------------------------------------------------------
class my1Memory : public my1Address
{
protected:
	bool mReadOnly, mProgramMode; // program mode allow write to read only!
	aword mLastUsed;
	abyte *mSpace;
public:
	my1Memory(int aStart=0x0, int aSize=MAX_MEMSIZE, bool aROM=false);
	virtual ~my1Memory();
	bool IsReadOnly(void);
	void ProgramMode(bool aStatus=true);
	int GetLastUsed(void); // gets address, NOT index!
	virtual bool ReadData(aword,abyte&);
	virtual bool WriteData(aword,abyte);
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
class my1BitIO : public my1SimObject
{
protected:
	bool mInput;
	abyte mState; // in case a tri-state device?
public:
	my1BitIO();
	virtual ~my1BitIO(){}
	bool IsInput(void);
	void SetInput(bool anInput=true);
	abyte GetState(void);
	void SetState(abyte);
	abyte GetData(void);
	void SetData(abyte);
};
//------------------------------------------------------------------------------
class my1DevicePort : public my1SimObject
{
protected:
	int mSize;
	my1BitIO *mDevicePins;
public:
	my1DevicePort(int aSize=0);
	virtual ~my1DevicePort();
	int GetSize(void);
	void SetSize(int);
	my1BitIO* GetBitIO(int);
	abyte IsInput(void);
	void SetInput(bool anInput=true);
	abyte GetPort(void);
	void SetPort(abyte);
	abyte GetData(void);
	void SetData(abyte);
};
//------------------------------------------------------------------------------
class my1Device : public my1Address // can act like a memory?
{
protected:
	my1DevicePort *mDevicePorts;
public:
	my1Device(int aStart=0x0, int aSize=MAX_DEVSIZE);
	virtual ~my1Device();
	my1DevicePort* GetDevicePort(int);
	virtual bool ReadDevice(abyte,abyte&);
	virtual bool WriteDevice(abyte,abyte);
	virtual bool ReadData(aword,abyte&);
	virtual bool WriteData(aword,abyte);
	// methods for 'external' device?
	virtual abyte GetData(int);
	virtual void PutData(int,abyte);
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
};
//------------------------------------------------------------------------------
class my1Reg85 : public my1SimObject
{
protected:
	aword mData;
	bool mDoubleSize;
	my1Reg85 *pLO, *pHI;
public:
	my1Reg85(bool aDoubleSize=false);
	virtual ~my1Reg85(){}
	void Use(my1Reg85* aReg=0x0, my1Reg85* bReg=0x0);
	bool IsDoubleSize(void);
	virtual aword GetData(void);
	virtual void SetData(aword);
	virtual aword Increment(void);
	virtual aword Decrement(void);
	virtual aword Accumulate(aword);
	my1Reg85& operator=(my1Reg85&);
};
//------------------------------------------------------------------------------
class my1Pin85 : public my1Reg85, public my1DevicePort
{
public: // specially designed for 8085 interrupt register
	my1Pin85();
	virtual ~my1Pin85(){}
	my1BitIO& RegBit(int);
	virtual aword GetData(void);
	virtual void SetData(aword);
};
//------------------------------------------------------------------------------
class my1Reg85Pair : public my1Reg85
{
public:
	my1Reg85Pair(my1Reg85* aReg=0x0, my1Reg85* bReg=0x0) : my1Reg85(true)
	{
		this->Use(aReg,bReg);
	}
	virtual ~my1Reg85Pair(){}
};
//------------------------------------------------------------------------------
class my1AddressMap : public my1SimObject
{
protected:
	my1Address* mObjects[MAX_ADDRMAP_COUNT]; // pointer list only!
	int mCount, mMapSize;
public:
	my1AddressMap();
	virtual ~my1AddressMap(){}
	int GetCount(void);
	int GetMapSize(void);
	// management functions
	int Insert(my1Address*,int anIndex=-1);
	my1Address* Remove(int anIndex=-1);
	my1Address* Object(aword);
	my1Address* Object(int);
	// pure-virtual functions
	virtual bool Read(aword,abyte&) = 0;
	virtual bool Write(aword,abyte) = 0;
};
//------------------------------------------------------------------------------
class my1MemoryMap85 : public my1AddressMap
{
public:
	my1MemoryMap85();
	virtual ~my1MemoryMap85(){}
	my1Memory* Memory(aword);
	void ProgramMode(bool aStatus=true);
	virtual bool Read(aword,abyte&);
	virtual bool Write(aword,abyte);
};
//------------------------------------------------------------------------------
class my1DeviceMap85 : public my1AddressMap
{
public:
	my1DeviceMap85();
	virtual ~my1DeviceMap85(){}
	my1Device* Device(aword);
	virtual bool Read(aword,abyte&);
	virtual bool Write(aword,abyte);
};
//------------------------------------------------------------------------------
class my1Sim8085 : public my1SimObject
{
protected:
	bool mErrorRW, mErrorISA; // used internally ONLY!
	bool mHalted, mIEnabled; // state representation?
	my1Reg85 mRegMAIN[I8085_REG_COUNT];
	my1Reg85Pair mRegPAIR[I8085_RP_COUNT], mRegPC, mRegPSW;
	my1Pin85 mRegINTR;
	my1MemoryMap85 mMemory;
	my1DeviceMap85 mDevice;
public:
	my1Sim8085();
	virtual ~my1Sim8085(){}
protected:
	abyte GetParity(abyte);
	abyte GetSrcData(abyte);
	void PutDstData(abyte,abyte);
	void DoStackPush(aword*);
	void DoStackPop(aword*);
	void UpdateFlag(abyte,abyte);
	bool CheckFlag(abyte);
protected:
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
	void ExecDelay(int);
public:
	int ExecCode(CODEX*); // returns machine state count!
	my1BitIO& Pin(int);
};
//------------------------------------------------------------------------------
class my1Sim85 : public my1Sim8085
{
protected:
	bool mReady, mBuilt;
	int mStartAddress;
	int mCodeCount;
	CODEX *mCodexList, *mCodexExec;
	// the default config!
	my1Sim2764 mROM;
	my1Sim6264 mRAM;
	my1Sim8255 mPPI;
public:
	my1Sim85(bool aDefaultConfig=false);
	virtual ~my1Sim85();
	int GetStartAddress(void);
	void SetStartAddress(int);
protected:
	// codex management
	bool FreeCodex(void);
	bool LoadCodex(char*);
	bool ResetCodex(void);
	void LoadStuff(STUFFS*);
	bool GetCodex(aword);
	bool ExeCodex(void);
	// memory/device interface
	bool ReadMemory(aword,abyte&);
	bool WriteMemory(aword,abyte);
	bool ReadDevice(abyte,abyte&);
	bool WriteDevice(abyte,abyte);
public:
	// simulation functions
	bool ResetSim(int aStart=0x0);
	bool StepSim(void);
	bool RunSim(int aStep=1);
	// memory/device management
	bool InsertMemory(my1Memory*,int anIndex=-1);
	bool InsertDevice(my1Device*,int anIndex=-1);
	my1Memory* RemoveMemory(int anIndex=-1);
	my1Device* RemoveDevice(int anIndex=-1);
	my1Memory* GetMemory(int);
	my1Device* GetDevice(int);
	// build function
	bool BuildDefault(void);
	bool BuildReset(void);
	// high-level sim interface
	bool Assemble(const char*);
	bool Save2HEX(const char*);
	bool Simulate(int aStep=1);
	// for external access
	my1Reg85* GetRegister(int);
	int GetMemoryCount(void);
	int GetDeviceCount(void);
	int GetCodexLine(void);
	void PrintCodexInfo(void);
};
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
