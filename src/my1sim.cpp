//------------------------------------------------------------------------------
#include "asimcomp.h"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ATimeSimObject::ATimeSimObject ( const char* aName, long aTime )
{
	mNext = 0x0;
	mName[0] = 0x0;
	if ( aName )
	{
		// copy name
		for ( int cLoop=0;cLoop<SIMNAMELIMIT;cLoop++ )
		{
			mName[cLoop] = aName[cLoop];
			if ( aName[cLoop]==0x0 )
				break;
		}
		mName[SIMNAMELIMIT-1] = 0x0; // security
	}
	mTimeStart = -1;
	mTimeNeeded = aTime;
	mTimeActive = 0;
	mTimeCount = 0;
	mCompleted = false;
	mActive = false;
}
//------------------------------------------------------------------------------
ATimeSimObject* ATimeSimObject::GetNext ( void )
{
	return mNext;
}
//------------------------------------------------------------------------------
void ATimeSimObject::SetNext ( ATimeSimObject* aNext )
{
	mNext = aNext;
}
//------------------------------------------------------------------------------
bool ATimeSimObject::IsName ( const char* aName )
{
	bool cThisIsIt = false;
	if ( aName )
	{
		cThisIsIt = true;
		// compare name
		for ( int cLoop=0;cLoop<SIMNAMELIMIT;cLoop++ )
		{
			if ( mName[cLoop]!=aName[cLoop] )
			{
				cThisIsIt = false;
				break;
			}
			if ( aName[cLoop]==0x0 )
				break;
		}
	}

	return cThisIsIt;
}
//------------------------------------------------------------------------------
long ATimeSimObject::GetStartTime ( void )
{
	return mTimeStart;
}
//------------------------------------------------------------------------------
long ATimeSimObject::GetNeededTime ( void )
{
	return mTimeNeeded;
}
//------------------------------------------------------------------------------
long ATimeSimObject::GetActiveTime ( void )
{
	return mTimeActive;
}
//------------------------------------------------------------------------------
long ATimeSimObject::GetCompleteTime ( void )
{
	return mTimeStart+mTimeCount;
}
//------------------------------------------------------------------------------
void ATimeSimObject::Activate ( void )
{
	mActive = true;
}
//------------------------------------------------------------------------------
void ATimeSimObject::DeActivate ( void )
{
	mActive = false;
}
//------------------------------------------------------------------------------
bool ATimeSimObject::IsActive ( void )
{
	return mActive;
}
//------------------------------------------------------------------------------
bool ATimeSimObject::IsCompleted ( void )
{
	return mCompleted;
}
//------------------------------------------------------------------------------
void ATimeSimObject::ExecuteUnit ( long aTime )
{
	if ( !mCompleted )
	{
		// only start counting when initiated
		if ( mTimeStart>=0 )
		{
			mTimeCount++;
			UserExecute();
		}
		// check complete time only when active
		if ( mActive )
		{
			if ( mTimeStart<0 )
			{
				mTimeStart = aTime;
			}
			else
			{
				mTimeActive++;
				if ( mTimeActive==mTimeNeeded )
				{
					mCompleted = true;
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
void ATimeSimObject::Reset ( void )
{
	mTimeStart = -1;
	mTimeActive = 0;
	mTimeCount = 0;
	mCompleted = false;
	mActive = false;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ATimeSimEvent::ATimeSimEvent ( long aTime,abyte aType,ATimeSimObject* aTimeSimObject )
{
	mNext = 0x0;
	mEventTime = aTime;
	mEventType = aType;
	mTimeSimObject = aTimeSimObject;
}
//------------------------------------------------------------------------------
ATimeSimEvent* ATimeSimEvent::GetNext ( void )
{
	return mNext;
}
//------------------------------------------------------------------------------
void ATimeSimEvent::SetNext ( ATimeSimEvent* aNext )
{
	mNext = aNext;
}
//------------------------------------------------------------------------------
long ATimeSimEvent::GetEventTime ( void )
{
	return mEventTime;
}
//------------------------------------------------------------------------------
abyte ATimeSimEvent::GetEventType ( void )
{
	return mEventType;
}
//------------------------------------------------------------------------------
ATimeSimObject* ATimeSimEvent::GetTimeSimObject ( void )
{
	return mTimeSimObject;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ATimeSimulation::ATimeSimulation()
{
	mTimeUnit = 0;
	mTimeComplete = 0;
	// link-list
	mObjectList = 0x0;
	mEventList = 0x0;
	mObjectCount = 0;
	mEventCount = 0;
}
//------------------------------------------------------------------------------
ATimeSimulation::~ATimeSimulation()
{
	ATimeSimObject* pObject = mObjectList;
	while ( pObject )
	{
		pObject = mObjectList->GetNext();
		delete mObjectList;
		mObjectList = pObject;
	}
	ATimeSimEvent* pEvent = mEventList;
	while ( pEvent )
	{
		pEvent = mEventList->GetNext();
		delete mEventList;
		mEventList = pEvent;
	}
}
//------------------------------------------------------------------------------
long ATimeSimulation::GetTimeUnit ( void )
{
	return mTimeUnit;
}
//------------------------------------------------------------------------------
long ATimeSimulation::TimeCompleted ( void )
{
	return mTimeComplete;
}
//------------------------------------------------------------------------------
void ATimeSimulation::Reset ( void )
{
	mTimeUnit = 0;
	mTimeComplete = 0;
	// reset all timesim objects
	ATimeSimObject* pObject = mObjectList;
	for ( int cLoop=0;cLoop<mObjectCount;cLoop++ )
	{
		pObject->Reset();
		pObject = mObjectList->GetNext();
	}
}
//------------------------------------------------------------------------------
bool ATimeSimulation::AddObject ( ATimeSimObject* anObject )
{
	bool cSuccess = false;
	ATimeSimObject* pObject = mObjectList;
	if ( pObject )
	{
		while ( pObject->GetNext() )
		{
			pObject = pObject->GetNext();
		}
		pObject->SetNext ( anObject );
	}
	else
	{
		mObjectList = anObject;
	}
	mObjectCount++;
	cSuccess = true;
	return cSuccess;
}
//------------------------------------------------------------------------------
bool ATimeSimulation::AddEvent ( ATimeSimEvent* anEvent )
{
	bool cSuccess = false;
	ATimeSimEvent* pEvent = mEventList;
	if ( pEvent )
	{
		// sort right away
		ATimeSimEvent* cEvent = 0x0; // previous item
		while ( anEvent->GetEventTime() <=pEvent->GetEventTime() )
		{
			cEvent = pEvent;
			pEvent = pEvent->GetNext();
		}
		if ( cEvent )
		{
			cEvent->SetNext ( anEvent );
		}
		else
		{
			mEventList = anEvent;
		}
		anEvent->SetNext ( pEvent );
	}
	else
	{
		mEventList = anEvent;
	}
	mEventCount++;
	cSuccess = true;
	return cSuccess;
}
//------------------------------------------------------------------------------
void ATimeSimulation::Run ( long aTimeUnits )
{
	bool cDone = false;
	bool cEventNow = false;
	ATimeSimObject* cObject;
	ATimeSimEvent* cEvent;

	long cTimeUnit = 0; // session time-unit
	while ( !cDone )
	{
		// pre-TU check
		TimeUnitCheck();
		// check all events
		while ( mEventCount>0 )
		{
			// get first event
			cEvent = mEventList;
			if ( cEvent->GetEventTime() ==mTimeUnit )
			{
				cObject = cEvent->GetTimeSimObject();
				cEventNow = true;
				switch ( cEvent->GetEventType() )
				{
					case EVENTBREAKPOINT:
						// breakpoint facility
						BreakPointCheck();
						break;
					case EVENTACTIVATEOBJ:
						cObject->Activate();
						break;
					case EVENTDEACTIVATEOBJ:
						cObject->DeActivate();
						break;
				}
				mEventList = cEvent->GetNext();
				delete cEvent;
				mEventCount--;
			}
			else
				break;
		}
		// process all objects
		cObject = mObjectList;
		while ( cObject )
		{
			cObject->ExecuteUnit ( mTimeUnit );
			cObject = cObject->GetNext();
		}
		// post-TU update
		TimeUnitUpdate();
		// local time count
		cTimeUnit++;
		if ( aTimeUnits>0 )
		{
			if ( cTimeUnit==aTimeUnits )
				cDone = true;
		}
		else if ( cEventNow )
			cDone = true;
	}
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
