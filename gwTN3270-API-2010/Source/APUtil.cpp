//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "APUtil.h"
#include <time.h>
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <ComObj.hpp>

//---------------------------------------------------------------------------
#pragma package(smart_init)

// 宣告 Global Comm Log 物件
ECommLog * g_CommLog ;

// 是否為 Debug Mode
bool gIsDebugMode  ;

// =========================================================================
// Private Functions
// =========================================================================


// =========================================================================
// Extern Functions
// =========================================================================

bool OpenSQL( AnsiString sSQL , TADOQuery * qry ) {
     qry->SQL->Text = sSQL ;
     qry->Open() ;
     return true ;
}

AnsiString GetBookmarkStringsByField( TDBGrid * dbGrid , TADOQuery * adoQuery , AnsiString sFieldName )
{
    int i ;
    AnsiString rcString ;

    rcString = "" ;

    for( i = 0 ; i < dbGrid->SelectedRows->Count ; i ++ ) {
        adoQuery->GotoBookmark((void *)dbGrid->SelectedRows->Items[i].c_str());

        if( rcString.Length() > 0 )
            rcString = rcString + "," ;

        rcString = rcString + "'" + adoQuery->FieldByName( sFieldName )->AsString  + "'" ;
    } // for
    
    return rcString ;
}

// 代換字串內容
char * QuickReplaceStr( char * Dest , char * sSource , char * sSearch , char * sReplaceWith )
{
    char sBuf[200] , *p , *descp ;
    int i , j , len , SearchLen , SearchIndex , DescIndex , ReplaceLen;
    len = strlen( sSource );
    SearchLen = strlen( sSearch );
    ReplaceLen = strlen( sReplaceWith );
    SearchIndex = 0 ;
    DescIndex = 0 ;
    for( i = 0 ; i < len ; i++ ) {
        if( sSource[i] == sSearch[SearchIndex] ) {
            sBuf[SearchIndex] = sSource[i] ;
            SearchIndex ++ ;
            if( SearchIndex == SearchLen ) {
                for( j = 0 ; j < ReplaceLen ; j++ ) {
                    Dest[DescIndex++] = sReplaceWith[j] ;
                } // for
                SearchIndex = 0 ;
            } // if
        } // if
        else {
            for( j = 0 ; j < SearchIndex ; j ++ ) {
                Dest[DescIndex++] = sBuf[j] ;
            } // for
            SearchIndex = 0 ;
            Dest[DescIndex++] = sSource[i] ;
        } // else
    } // for
    Dest[DescIndex] = '\0' ;

    return Dest;
}


// 處理 SQL 字串內容
AnsiString ParamStr( AnsiString sParam )
{
    AnsiString sRC ;
    char sDesc[40960] ;
    sRC = QuickReplaceStr( sDesc , sParam.Trim().c_str() , "'" , "''" );
    return sRC ;
}

// 處理 SQL 數值內容
AnsiString ParamInt( AnsiString sParam )
{
    AnsiString sRC ;
    sRC = IntToStr( sParam.ToIntDef(0) ).Trim();
    return sRC ;
}


// 處理 Excel 字串內容
AnsiString ExcelStr( AnsiString sStr )
{
    return AnsiString("=(\"") + sStr + "\")" ;

}



// 組合檔案路徑
AnsiString CheckFilePath( char * path , char *filename )
{
    AnsiString FileName ;

    if ( path[ strlen(path)-1 ] == '\\' )
        FileName = AnsiString( path ) + filename ;
    else {
        try{
            FileName = AnsiString( path ) + "\\" + filename ;
        }
        catch(Exception &E) {
            g_CommLog->WriteLog( "[CheckFilePath Exception]: %s" , E.Message.c_str() );
            FileName = "" ;
        }
    }
    return FileName ;
}

//將字串前補0
AnsiString FillZero(AnsiString sSrc, int DestLen)
{
    AnsiString sDest = sSrc;
    for ( int i = sSrc.Length(); i < DestLen; i++ )
        sDest = "0" + sDest;

    return sDest;
}

//將西元年格式 yyyy/mm/dd 轉換成民國年格式 yymmdd
AnsiString ChangeDateFmt(AnsiString sDate)
{
    return IntToStr(StrToInt(sDate.SubString(1, 4)) - 1911) + sDate.SubString(6, 2) + sDate.SubString(9, 2);
}

AnsiString GetDateStyle(AnsiString sDay)
{
    return sDay.SubString(1,4) + "/" + sDay.SubString(5,2)+ "/" + sDay.SubString(7,2);
}

AnsiString GetTimeStyle(AnsiString sTime)
{
    return sTime.SubString(1,2) + ":" +sTime.SubString(3,2);
}

AnsiString CheckFileNameNum(AnsiString sFileName)
{
        int     i;
        AnsiString New_FileName;
        for(i=1;i<=9999999;i++)
        {
                New_FileName = sFileName + "_is_the_Same_" + i +".txt";
                if(FileExists(New_FileName)==false)
                        break;
        }
        return New_FileName;
}

int LoadFromUniCodeFile(TStrings* ss, const String& fname)
{
    wchar_t *wbuff;
    char *abuff;
    wchar_t ft;
    int fsize = 0;

    ss->Clear();
    TFileStream* fs = new TFileStream(fname, fmOpenRead);
    if (fs->Read(&ft, sizeof(ft)) > 0) {
        if (ft != 0xFEFF) { delete fs; g_CommLog->WriteLog("%s isn't unicode file", fname.c_str()); }
        fsize = fs->Size;
        wbuff = new wchar_t[fsize / 2];
        abuff = new char[fsize];
        memset(wbuff, 0, fsize);
        memset(abuff, 0, fsize);
        if (fs->Read(wbuff, fsize)) {
            WideCharToMultiByte(CP_ACP, 0, wbuff, -1, abuff, fsize, NULL, NULL);
            ss->Text = abuff;
        }
        delete []wbuff;
        delete []abuff;
        delete fs;
    }
    return ss->Text.Length();
}

void SaveToUniCodeFile(TStrings* ss, const String& fname)
{
    wchar_t wf = 0xFEFF;
    TFileStream* fs= new TFileStream(fname, fmCreate);
    fs->Write(&wf, sizeof(wchar_t));
    WideString content = ss->Text;
    fs->Write(content.c_bstr(), content.Length() * sizeof(wchar_t));
    delete fs;
}

// =========================================================================
// CommLog Class
// =========================================================================

// 建構
ECommLog::ECommLog( AnsiString sSystem , char * sLogPath )
{
	int rc , nLen ;
    m_WriteToFile = true ;
    m_servicecount = 0 ;
    m_System = sSystem ;
	rc = GetFileAttributes( sLogPath ) ;

	// 判斷目錄存不存在
	if( ( rc != -1 ) && ( ( rc & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) ) {
		strcpy( m_LogPath , sLogPath ) ;
	} // if
	else if( CreateDir( sLogPath ) ) {
		strcpy( m_LogPath , sLogPath ) ;
	} // else
	else {
		GetCurrentDirectory( sizeof( m_LogPath ) , m_LogPath ) ;
	} // else

	// 補上目錄的最後一個 '\'
	nLen = strlen( m_LogPath );
	if( m_LogPath[nLen-1] != '\\' )
		strcat( m_LogPath , "\\" );
}

// 解構
ECommLog::~ECommLog()
{

}

// 建立目錄
bool ECommLog::CreateDir( char * sFilePath )
{
	bool rc = false ;
	char sTempPath[_MAX_PATH] , sCreatePath[255] ;
	int sPos ;

	try {
		strcpy( sTempPath , sFilePath ) ;
		sPos = strlen( sTempPath ) - 1 ;
		strcpy( sCreatePath , sTempPath ) ;

		// 找到 '\' 字元
		while( (sPos >= 0 ) && ( sTempPath[sPos] != '\\' ) ) sPos-- ;
		// 取得目錄字串
		if( sPos > 0 ) {
			sTempPath[sPos] = '\0' ;
			CreateDir( sTempPath ) ;
            // 建立目錄
            rc = !_mkdir( sCreatePath ) ;
		} // else


	} // try
	catch( Exception & E ) {
        throw ;
	} // catch

	return rc ;
}
/*
// 寫入 Log
bool ECommLog::WriteLog( char * fmt , ... )
{
	bool rc = false ;
    va_list     parg;
    FILE       *elog_fp;
	char sFileName[255] ;
	struct _SYSTEMTIME SysTime ;


	try {
		// 取得目前時間的 tm 結構
		if( (elog_fp=fopen( GetFileName.c_str() ,"at")) != NULL ) {
			// 寫入時間
			fprintf( elog_fp , "[%02d:%02d:%02d]\t" , SysTime.wHour , SysTime.wMinute , SysTime.wSecond ) ;
			// 寫入 Log String
			va_start(parg, fmt);
			vfprintf(elog_fp, fmt, parg);
			va_end(parg);
			// 加入換行字元
			fprintf( elog_fp , "\n") ;
			fclose(elog_fp);
			rc = true ;
		} // if
	} // try
	catch( Exception & E ) {
		throw ;
	} // catch

	return rc ;
}  */

AnsiString GetVersionStr( void )
{
    AnsiString sVerStr;
    DWORD dwVerInfoSize = 0;
    AnsiString szFile = Application->ExeName ;
    dwVerInfoSize = GetFileVersionInfoSize(szFile.c_str(), &dwVerInfoSize);
    BYTE *bVerInfoBuf = new BYTE[dwVerInfoSize];
    try
    {
        if (GetFileVersionInfo(szFile.c_str(), 0, dwVerInfoSize, bVerInfoBuf))
        {
            VS_FIXEDFILEINFO *vsInfo;
            UINT vsInfoSize;
            if (VerQueryValue(bVerInfoBuf, "\\", (void**)&vsInfo, &vsInfoSize))
            {
                int iFileVerMajor   = HIWORD(vsInfo->dwFileVersionMS);
                int iFileVerMinor   = LOWORD(vsInfo->dwFileVersionMS);
                int iFileVerRelease = HIWORD(vsInfo->dwFileVersionLS);
                int iFileVerBuild   = LOWORD(vsInfo->dwFileVersionLS);
                sVerStr.sprintf( " (Version: %d.%d.%d.%d)",
                				iFileVerMajor,
                                iFileVerMinor,
		                        iFileVerRelease,
                                iFileVerBuild );
            } // if
        } // if
    } // try
    catch( Exception & E )
    {
//        OutputEventLog( AnsiString(" GetVersionStr fail! ") + E.Message );
    } // catch

    delete [] bVerInfoBuf ;

    return sVerStr ;
}
