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
#define MAX_MEMCOUNT 8
#define MAX_DEVCOUNT 16
#define MAX_DEVNAME_CHAR 32
#define I8255_PORTA 0
#define I8255_PORTB 1
#define I8255_PORTC 2
#define I8255_PORTC_U 2
#define I8255_PORTC_L 3
#define I8255_CONTROL 3
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
#define I8085_RPPSW 3
//------------------------------------------------------------------------------
class my1Memory
{
	protected:
		aword mStart, mSize;
		abyte *mSpace;
	public:
		my1Memory(int aStart=0x0, int aSize=MAX_MEMSIZE);
		virtual ~my1Memory();
		int GetStart(void);
		int GetSize(void);
		abyte ReadData(aword);
		bool WriteData(aword,abyte);
		bool IsSelected(aword);
		bool IsWithin(int,int);
		bool IsWithin(my1Memory*);
};
//------------------------------------------------------------------------------
class my1Device : public my1Memory // acts like a memory?
{
	protected:
		char mName[MAX_DEVNAME_CHAR];
		abyte ReadData(aword anAddress);
		bool WriteData(aword anAddress, abyte aData);
	public:
		my1Device(char *aName, int aStart=0x0, int aSize=MAX_DEVSIZE);
		virtual abyte ReadDevice(aword anAddress);
		virtual bool WriteDevice(aword anAddress, abyte aData);
};
//------------------------------------------------------------------------------
class my1Sim8255 : public my1Device
{
	protected:
		bool mIsInput[4];
	public:
		my1Sim8255(int aStart=0x0);
		abyte GetData(int);
		void PutData(int anIndex, abyte aData, abyte aMask);
		virtual abyte ReadDevice(aword anAddress);
		virtual bool WriteDevice(aword anAddress, abyte aData);
};
//------------------------------------------------------------------------------
class my1Sim85
{
	protected:
		bool mHalted;
		abyte mFullReg[8];
		aword mPCounter;
		aword mSPointer;
		int mMemCount, mDevCount, mCodCount, mStateExec;
		my1Memory* mMems[MAX_MEMCOUNT];
		my1Device* mDevs[MAX_DEVCOUNT];
		CODEX *mCodexList, *mCodexExec;
		void LoadStuff(STUFFS*);
		bool GetCodex(aword);
		bool ExeDelay(void);
		bool ExeCodex(void);
		abyte GetSrcData(abyte);
		void ExecMOV(abyte, abyte);
		void ExecMOVi(abyte, abyte);
		void ExecALU(abyte, abyte);
		void ExecALUi(abyte, abyte);
		void ExecLXI(abyte, aword);
		void ExecDAD(abyte);
		void ExecSTAXLDAX(abyte);
	public:
		my1Sim85();
		virtual ~my1Sim85();
		void (*DoDelay)(void*); // public function pointer
		int GetStateExec(void);
		int GetCodexLine(void);
		// simulation setup
		bool AddMemory(my1Memory*);
		bool AddDevice(my1Device*);
		my1Memory* GetMemory(int);
		my1Device* GetDevice(int);
		bool ReadMemory(aword,abyte&);
		bool WriteMemory(aword,abyte);
		bool ResetCodex(void);
		bool LoadCodex(char*);
		// simulation functions
		bool ResetSim(void);
		bool StepSim(void);
		bool RunSim(int);
};
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
