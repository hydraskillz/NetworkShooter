/*
 * Author      : Matthew Johnson
 * Date        : 21/Sep/2013
 * Description :
 *   
 */
 
#pragma once

namespace mage
{

	class NetSession;
	class NetClient
	{
	public:
		friend class NetSession;

		NetClient();
		~NetClient();

		clientID_t GetID() const					{ return mID; }
		void SetSession( NetSession* session )		{ mSession = session; }

		IPaddress Address;

	protected:
		clientID_t mID;				// Unique network ID
		NetSession* mSession;		// Session we belong to
	};

}