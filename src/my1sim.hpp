//------------------------------------------------------------------------------
// Time-Simulation Components
//------------------------------------------------------------------------------
#ifndef __ASIMCOMPHPP
#define __ASIMCOMPHPP
//------------------------------------------------------------------------------
#define SIMNAMELIMIT 40 // object's name is usually short :p
#define EVENTBREAKPOINT 0x0
#define EVENTACTIVATEOBJ 0x1
#define EVENTDEACTIVATEOBJ 0x2
//------------------------------------------------------------------------------
#include "absccomp.h"
//------------------------------------------------------------------------------
class ATimeSimObject : public AObject
{
	protected:
		ATimeSimObject* mNext; // for link-list
		char mName[SIMNAMELIMIT]; // object name
		long mTimeStart; // time started (-1 => NOT!)
		long mTimeNeeded; // time needed to complete
		long mTimeActive; // active time
		long mTimeCount; // total time taken
		bool mCompleted; // completed status
		bool mActive; // active status
	public:
		ATimeSimObject ( const char*,long ); // name, time-needed
		virtual ~ATimeSimObject() {}
		ATimeSimObject* GetNext ( void );
		void SetNext ( ATimeSimObject* );
		bool IsName ( const char* ); // must be null-terminated
		long GetStartTime ( void );
		long GetNeededTime ( void );
		long GetActiveTime ( void );
		long GetCompleteTime ( void );
		void Activate ( void );
		void DeActivate ( void );
		bool IsActive ( void );
		bool IsCompleted ( void );
		void ExecuteUnit ( long ); // current sim-time
		void Reset ( void );
		// processing function for derived class
		virtual void UserExecute ( void ) {} // called even when NOT active
};
//------------------------------------------------------------------------------
class ATimeSimEvent : public AObject
{
	protected:
		ATimeSimEvent* mNext;
		long mEventTime;
		abyte mEventType;
		ATimeSimObject* mTimeSimObject;
	public:
		ATimeSimEvent ( long,abyte,ATimeSimObject* ); // time, type, object
		virtual ~ATimeSimEvent() {}
		ATimeSimEvent* GetNext ( void );
		void SetNext ( ATimeSimEvent* );
		long GetEventTime ( void );
		abyte GetEventType ( void );
		ATimeSimObject* GetTimeSimObject ( void );
};
//------------------------------------------------------------------------------
class ATimeSimulation : public AObject
{
	protected:
		long mTimeUnit;
		long mTimeComplete;
		// link-list
		ATimeSimObject* mObjectList;
		ATimeSimEvent* mEventList;
		int mObjectCount;
		int mEventCount;
	public:
		ATimeSimulation();
		virtual ~ATimeSimulation();
		long GetTimeUnit ( void );
		long TimeCompleted ( void );
		void Reset ( void );
		bool AddObject ( ATimeSimObject* );
		bool AddEvent ( ATimeSimEvent* );
		void Run ( long );  // if <=0, run till next event
		// make this virtual NOT pointers - can access members
		// always override this in subclasses when needed
		virtual void BreakPointCheck ( void ) {} // process breakpoint requests
		virtual void TimeUnitCheck ( void ) {} // pre-TU processing check
		virtual void TimeUnitUpdate ( void ) {} // post-TU update
};
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
