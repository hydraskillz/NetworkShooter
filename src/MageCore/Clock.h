/*
 * Author      : Matthew Johnson
 * Date        : 29/May/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class Clock
	{
	private:
		Clock();
		~Clock();
	public:
		enum TimeFormat
		{
			TIME_SEC,
			TIME_MILLI,
			TIME_MICRO
		};

		// Initialize the clock system and return the master clock
		static Clock& Initialize();
		// Get the master clock. Calls Initialize() if needed.
		static Clock& GetMasterClock();
		// Creates and registers a new clock
		static Clock* CreateClock( Clock* parent );
		// Destroy an existing clock
		static void DestroyClock( Clock*& clock );

		inline double GetElapsedTime( TimeFormat timeFormat ) const;
		inline double GetDeltaTime( TimeFormat timeFormat ) const;
		inline double GetTimeScale() const;

		inline bool IsPaused() const;

		void SetTimeScale( double timeScale );
		void AdvanceTime( double deltaSeconds );
		void SetMaxDeltaSeconds( double deltaSeconds );
		void SetTime( double timeSeconds );

		void Pause();
		void Resume();

		void PostEventCallbackAt( const std::string& eventName, double timeToFireSecods, const Dictionary& params=Dictionary() );
		void PostEventCallbackAfter( const std::string& eventName, double delaySeconds, const Dictionary& params=Dictionary() );

		static double GetFormatedTime( TimeFormat timeFormat, double seconds );
		// Returns time in hh:mm.ss.mmm
		static std::string GetStringFormatedTime( double seconds );
	private:
		
		double mElapsedSeconds;
		double mDeltaSeconds;
		double mMaxDeltaSeconds;
		double mTimeScale;
		bool mIsPaused;

		Clock* mParent;
		std::vector< Clock* > mChildren;

		struct ClockTimeEventCallback
		{
			std::string EventName;
			double TimeToFireSeconds;
			Dictionary params;

			bool operator<( const ClockTimeEventCallback& other )
			{
				return TimeToFireSeconds < other.TimeToFireSeconds;
			}
		};

		std::vector< ClockTimeEventCallback > mTimeEvents;

		static Clock* msMasterClock;
		static const double DEFAULT_MAX_DELTA_SECONDS;


		// Platform specific
		class ClockPDI* mClockPDI;
	public:
		// Query real time
		void BeginTimeQuery();
		void EndTimeQuery();
		double QueryTimeElapsed( TimeFormat timeFormat=TIME_SEC );
		static double QueryTime( TimeFormat timeFormat=TIME_SEC );

	};

	class ClockPDI
	{
	public:
		// Query real time
		virtual void BeginTimeQuery() = 0;
		virtual void EndTimeQuery() = 0;
		virtual double QueryTimeElapsed( Clock::TimeFormat timeFormat ) = 0;
		static double QueryTime( Clock::TimeFormat timeFormat );
	};

#include "Clock.hpp"

}