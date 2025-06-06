// EhllapiImpl.h: interface for the EhllapiImpl class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __EHLLAPIIMPL_H__
#define __EHLLAPIIMPL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <exception>
#include <string>
#include "EhllapiError.h"

#define CGS_FIELD_ATTR_C0 64
#define CGS_FIELD_ATTR_C1 193
#define CGS_FIELD_ATTR_E0 96
#define CGS_FIELD_ATTR_E8 232
#define CGS_FIELD_ATTR_F0 240
#define CGS_FIELD_ATTR_F1 241
#define CGS_FIELD_ATTR_F8 248
#define CGS_FIELD_ATTR_FC 124
#define CGS_FIELD_ATTR_ERR 217
#define CGS_FIELD_ATTR_NORMAL 80

namespace HACL	// Host Access Class Library
{
	typedef long (WINAPI *_vx)(LPINT, LPSTR, LPINT, LPINT);

	class EhllapiImpl  
	{
	private:
		//static HINSTANCE hLib;
		//static _vx vx;
        DWORD prevtick ;
        HINSTANCE hLib;
        _vx vx ;
        char szTempDllName[255] ;
        char szDllFileName[255] ;
		bool m_IsConnect;

	protected:
        char m_SessionStr[20];
		char m_SessionId;
		int m_nFuncCode;	// Function Code
		int m_nLength;
		int m_nRetCode;		// Return Code
	public:
		bool m_nCommError;
		char m_szOIA[512];
		char m_szStr[8192];
        AnsiString LastErrMsg ;

	private:
		// call hllapi
		int inline call_api();
		// CONNECT_PS
		bool connect();

	public:
		// SET_SESSION_PARMS
		void SetParams(const char *param);

	public:
		// constructor
		EhllapiImpl();
		// destructor
		virtual ~EhllapiImpl();
		// COPY_OIA
		int CopyOIA();
		bool IsConnect();
		// WAIT
		int WaitForInput();
		// SEARCH_PS
		int SearchString(const char* lpstr, int row, int col);
		int SearchString(const char* lpstr, int pos);
		bool MatchString(const char *lpstr, int row, int col);
		bool MatchString(const char *lpstr, int pos);
		// COPY_PS_TO_STR
		void GetString(char* lpstr, int length);
		void GetString(char* lpstr, int length, int row, int col);
		void GetString(char* lpstr, int length, int pos);
		// COPY_PS
		int CopyPS();
		// COPY_STR_TO_PS
		void SetString(const string& str, int row, int col);
		void SetString(const char* lpstr, int row, int col);
		void SetString(const char* lpstr, int pos);
		// SENDKEY
		void SendKeys(const char* lpstr, int row, int col);
		void SendKeys(const char* lpstr, int pos);
		void SendKeys(const char* lpstr);
		// QUERY_CURSOR_LOC
		int GetCursorPos();
		// SET_CURSOR
		void SetCursorPos(int row, int col);
		void SetCursorPos(int pos);
		// CONVERT ROW&COLUMN TO POSITION VALUE
		int ConvertRowColToPos(int row, int col);
		// DISCONNECT_PS
		void Disconnect();
		// call connect()
		bool Connect();
		bool Connect(char *id);
		char SessionId();
		void SessionId(char id);
		// QUERY_SYSTEM
		void QuerySystem(void);
		// START_HOST_NOTIFY
		void StartHostNotify(char param);
		// QUERY_HOST_UPDATE
		int QueryHostUpdate(void);
		// STOP_HOST_NOTIFY
		void StopHostNotify(void);
		// QUERY_SESSION_STATUS
		void QuerySessionStatus(void);
		// QUERY_SESSIONS
		void QuerySessions(void);
		// RESERVE
		void LockKeyboard(void);
		// RELEASE
		void UnlockKeyboard(void);
		// QUERY_FIELD_ATTR
		int FieldAttribute(int row, int col);
		// Initialize api library
		static bool Initialize( char * dllpath );
		// release api library
		static void UnInitialize(void);
	};
}	// namespace HACL
#endif // __EHLLAPIIMPL_H__
