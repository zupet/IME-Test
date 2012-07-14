// TInput.cpp - part if Multi-Language input class
//
// Copyright (C) 2000-2005, Kwon-il Lee
//
// Kwon-il Lee
// zupet@hitel.net


#include "stdafx.h"
#include "TInput.h"
#include "TImm.h"

/*----------------------------------------------------------------------------*/ 

#define CHT_IMEFILENAME1    "TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2    "CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3    "MSTCIPHA.IME" // Phonetic 5.1
#define CHS_IMEFILENAME1    "PINTLGNT.IME" // MSPY1.5/2/3
#define CHS_IMEFILENAME2    "MSSCIPYA.IME" // MSPY3 for OfficeXP

#define LANG_CHT            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS            MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)
#define _CHT_HKL            ( (HKL)(INT_PTR)0xE0080404 ) // New Phonetic
#define _CHT_HKL2           ( (HKL)(INT_PTR)0xE0090404 ) // New Chang Jie
#define _CHS_HKL            ( (HKL)(INT_PTR)0xE00E0804 ) // MSPY
#define MAKEIMEVERSION( major, minor ) \
    ( (DWORD)( ( (BYTE)( major ) << 24 ) | ( (BYTE)( minor ) << 16 ) ) )

#define IMEID_CHT_VER42 ( LANG_CHT | MAKEIMEVERSION( 4, 2 ) )	// New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43 ( LANG_CHT | MAKEIMEVERSION( 4, 3 ) )	// New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44 ( LANG_CHT | MAKEIMEVERSION( 4, 4 ) )	// New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50 ( LANG_CHT | MAKEIMEVERSION( 5, 0 ) )	// New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51 ( LANG_CHT | MAKEIMEVERSION( 5, 1 ) )	// New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52 ( LANG_CHT | MAKEIMEVERSION( 5, 2 ) )	// New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60 ( LANG_CHT | MAKEIMEVERSION( 6, 0 ) )	// New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHS_VER41	( LANG_CHS | MAKEIMEVERSION( 4, 1 ) )	// MSPY1.5	// SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42	( LANG_CHS | MAKEIMEVERSION( 4, 2 ) )	// MSPY2	// Win2k/WinME
#define IMEID_CHS_VER53	( LANG_CHS | MAKEIMEVERSION( 5, 3 ) )	// MSPY3	// WinXP

enum { INDICATOR_NON_IME, INDICATOR_CHS, INDICATOR_CHT, INDICATOR_KOREAN, INDICATOR_JAPANESE };
enum { IMEUI_STATE_OFF, IMEUI_STATE_ON, IMEUI_STATE_ENGLISH };

#define LCID_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

wchar_t		g_aszIndicator[5][3] =  
{
	L"En",
	L"\x7B80",
	L"\x7E41",
	L"\xAC00",
	L"\x3042",
};

static TImm g_imm;

/*----------------------------------------------------------------------------*/ 
int		ConvertString(UINT codePage, const wchar_t* wText, int wLen, char* text, int len)
{
	if(text == 0) 
	{
		return WideCharToMultiByte(codePage, 0, wText, wLen, NULL, 0, NULL, NULL);
	}
	else 
	{
		int tLen = WideCharToMultiByte(codePage, 0, wText, wLen, NULL, 0, NULL, NULL);

		if(tLen > len) 
		{
			return 0;
		}
		else 
		{
			return WideCharToMultiByte(codePage, 0, wText, wLen, text, tLen, NULL, NULL);
		}
	}
}

/*----------------------------------------------------------------------------*/ 
int		ConvertString(UINT codePage, const char* text, int len, wchar_t* wText, int wLen)
{
	if(text == 0) 
	{
		return MultiByteToWideChar(codePage, 0, NULL, 0, wText, wLen);
	}
	else 
	{
		int tLen = MultiByteToWideChar(codePage, 0, NULL, 0, wText, wLen);

		if(tLen > len) 
		{
			return 0;
		}
		else 
		{
			return MultiByteToWideChar(codePage, 0, text, tLen, wText, wLen);
		}
	}
}

/*----------------------------------------------------------------------------*/ /* Begin of TInput */ 
TInput::TInput()
:	m_ulStart(0)
,	m_ulEnd(0)
,	m_hDllIme(NULL)
,	m_ImeState(IMEUI_STATE_OFF)
,	m_ptim(NULL)
{
	HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	if ( SUCCEEDED( hr ) )
	{
		hr = CoCreateInstance( CLSID_TF_ThreadMgr,
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(ITfThreadMgr),
			(void**)&m_ptim );
	}
}

/*----------------------------------------------------------------------------*/ 
TInput::~TInput()
{
	if( m_hDllIme ) 
		FreeLibrary( m_hDllIme );

	if(m_ptim)
		m_ptim->Release();

	CoUninitialize();
}

/*----------------------------------------------------------------------------*/ 
void	TInput::InitInput()
{
	g_imm.Init();
}

/*----------------------------------------------------------------------------*/ 
void	TInput::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
		OnInputLanguageChange(hWnd, wParam, lParam);

		//if ( m_ptim )
		//{
		//	ITfDocumentMgr* pdimPrev;
		//	if ( SUCCEEDED( m_ptim->AssociateFocus( hWnd, NULL, &pdimPrev ) ) )
		//	{
		//		if ( pdimPrev )
		//			pdimPrev->Release();
		//	}
		//}
}

/*----------------------------------------------------------------------------*/ 
void	TInput::OnInputLanguageChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hkl = (HKL)lParam;
	m_langId = LOWORD(m_hkl);
	m_codePage = GetCodePageFromLang(m_langId);
	m_input.resize(0);

	/* Check Property */ 
	DWORD property = ImmGetProperty(GetKeyboardLayout(0), IGP_PROPERTY);

	m_bUnicodeIME = (property & IME_PROP_UNICODE) ? true : false;

	/* Update Indicator */ 
	CheckToggleState(hWnd);

	/* Update m_dwId[] */ 
	GetImeId();

	/* Bind Proc */ 
	SetupImeApi(hWnd);
}

/*----------------------------------------------------------------------------*/ 
bool	TInput::OnComposition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HIMC hImc;
	if(lParam&GCS_COMPSTR) 
	{
		hImc = ImmGetContext(hWnd);
		if(hImc) 
		{ 

			int tempSize = ImmGetCompositionStringW(hImc, GCS_COMPSTR, NULL, 0);

			wchar_t* temp = (wchar_t*)alloca(tempSize);

			ImmGetCompositionStringW(hImc, GCS_COMPSTR, temp, tempSize);

			m_comp.assign(temp, temp+tempSize/sizeof(wchar_t));

			ImmReleaseContext(hWnd, hImc);
		}
	}
	if(lParam&GCS_RESULTSTR) 
	{
		hImc = ImmGetContext(hWnd);
		if(hImc) 
		{ 
			int tempSize = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0);

			wchar_t* temp = (wchar_t*)alloca(tempSize);

			ImmGetCompositionStringW(hImc, GCS_RESULTSTR, temp, tempSize);

			m_input.append(temp, temp+tempSize/sizeof(wchar_t));

			ImmReleaseContext(hWnd, hImc);
		}
	}
	if(lParam&GCS_COMPATTR) 
	{
		hImc = ImmGetContext(hWnd);
		if(hImc)
		{ 
			int tempSize = ImmGetCompositionStringW(hImc, GCS_COMPATTR, NULL, 0);

			BYTE* temp = (BYTE*)alloca(tempSize);

			ImmGetCompositionStringW(hImc, GCS_COMPATTR, temp, tempSize);

			int start, end;
			for(start=0; start<tempSize; ++start) if(temp[start]==ATTR_TARGET_CONVERTED || temp[start]==ATTR_TARGET_NOTCONVERTED) break;
			for(end=start; end<tempSize; ++end) if(temp[end]!=temp[start]) break;

			m_ulStart	= start;
			m_ulEnd		= end;

			ImmReleaseContext(hWnd, hImc);
		}
	}

	return true;
}

/*----------------------------------------------------------------------------*/ 
bool	TInput::OnEndComposition(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_comp.resize(0);
	m_ulStart = m_ulEnd = 0;
	m_reading.resize(0);

	return true;
}

/*----------------------------------------------------------------------------*/ 
bool	TInput::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	TCHAR temp[256];
	_stprintf_s(temp, TEXT("%08x\r\n"), wParam);
	OutputDebugString(temp);

	HIMC hImc;

	switch (wParam) 
	{
	case IMN_OPENCANDIDATE:
	case IMN_CHANGECANDIDATE:
		hImc = ImmGetContext(hWnd);
		if(hImc) 
		{ 
			m_reading.resize(0);

			int candidateLen = ImmGetCandidateListW(hImc, 0, NULL, 0);

			if(candidateLen > 0)
			{ 
				m_candidate.resize(candidateLen);

				ImmGetCandidateListW(hImc, 0, (CANDIDATELIST*)&m_candidate[0], candidateLen);
			}

			ImmReleaseContext(hWnd, hImc);
		}
		return true;

	case IMN_CLOSECANDIDATE:
		m_candidate.resize(0);
		return true;

	case IMN_SETCONVERSIONMODE:
	case IMN_SETOPENSTATUS:
        CheckToggleState(hWnd);
		return false;

    case IMN_PRIVATE:
        GetPrivateReadingString(hWnd);

        // Trap some messages to hide reading window
        switch( m_dwId[0] )
        {
            case IMEID_CHT_VER42:
            case IMEID_CHT_VER43:
            case IMEID_CHT_VER44:
            case IMEID_CHS_VER41:
            case IMEID_CHS_VER42:
				if((lParam==1)||(lParam==2)) return true;
                break;

            case IMEID_CHT_VER50:
            case IMEID_CHT_VER51:
            case IMEID_CHT_VER52:
            case IMEID_CHT_VER60:
            case IMEID_CHS_VER53:
                if((lParam==16)||(lParam==17)||(lParam==26)||(lParam==27)||(lParam==28)) return true;
                break;
        }
        break;
	}

	return false;
}

/*----------------------------------------------------------------------------*/ 
bool	TInput::OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) 
	{
	case '\r':
	case '\n':
		m_input.resize(0);
		break;
	case '\b':
		if(m_input.size()) m_input.resize(m_input.size() - 1);
		break;
	case '\t':
	case 27:
		break;
	default:
		if(wParam > 31) 
		{
			wchar_t temp;

			MultiByteToWideChar(m_codePage, 0, (char*)&wParam, 1, &temp, 1);

			m_input.push_back(temp);
		}
		break;
	}

	return true;
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetInput(wchar_t* text, int len)
{
	wcsncpy_s(text, len, m_input.c_str(), m_input.size());

	return min(len, (int)m_input.size());
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetComp(wchar_t* text, int len)
{
	wcsncpy_s(text, len, m_comp.c_str(), m_comp.size());

	return min(len, (int)m_comp.size());
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetReading(wchar_t* text, int len)
{
	wcsncpy_s(text, len, m_reading.c_str(), m_reading.size());

	return min(len, (int)m_reading.size());
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetCandidate(DWORD index, wchar_t* text, int len)
{
	if(m_candidate.empty()) 
	{
		return 0;
	}
	else 
	{
		CANDIDATELIST* candidateList = (CANDIDATELIST*)&m_candidate[0];
		if(index >= candidateList->dwCount)
		{
			return 0;
		}
		else 
		{
			if(m_bUnicodeIME)
			{
				
				wchar_t* wText = (wchar_t*)(&m_candidate[0] + candidateList->dwOffset[index]);

				wcsncpy_s(text, len, wText,wcslen(wText));

				return min(len, (int)wcslen(wText));
			}
			else 
			{
				char* temp = (char*)(&m_candidate[0] + candidateList->dwOffset[index]);
				int tempLen = strlen(temp);

				return ConvertString(m_codePage, temp, tempLen, text, len);
			}
		}
	}
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetInput(char* text, int len)
{
	return ConvertString(m_codePage, m_input.c_str(), m_input.size(), text, len);
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetComp(char* text, int len)
{
	return ConvertString(m_codePage, m_comp.c_str(), m_comp.size(), text, len);
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetReading(char* text, int len)
{
	return ConvertString(m_codePage, m_reading.c_str(), m_reading.size(), text, len);
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetCandidate(DWORD index, char* text, int len) 
{
	if(m_candidate.empty()) 
	{
		return 0;
	}
	else 
	{
		CANDIDATELIST* candidateList = (CANDIDATELIST*)&m_candidate[0];

		if(index >= candidateList->dwCount)
		{
			return 0;
		}
		else 
		{
			if(m_bUnicodeIME)
			{
				
				wchar_t* wText = (wchar_t*)(&m_candidate[0] + candidateList->dwOffset[index]);

				return ConvertString(m_codePage, wText, wcslen(wText), text, len);

			}
			else 
			{

				char* temp = (char*)(&m_candidate[0] + candidateList->dwOffset[index]);
				int tempLen = strlen(temp);

				if(len < tempLen)
				{
					return 0;
				}
				else 
				{
					memcpy(text, temp, tempLen);
					return tempLen;
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetCandidateCount() 
{
	if(m_candidate.empty()) 
	{
		return 0;
	}
	else 
	{
		return ((CANDIDATELIST*)&m_candidate[0])->dwCount;
	}
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetCandidateSelection() 
{
	if(m_candidate.empty()) 
	{
		return 0;
	}
	else
	{
		if(PRIMARYLANGID(m_langId) == LANG_KOREAN)
			return ((CANDIDATELIST*)&m_candidate[0])->dwCount;
		else
			return ((CANDIDATELIST*)&m_candidate[0])->dwSelection;
	}
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetCandidatePageSize() 
{
	if(m_candidate.empty()) 
	{
		return 0;
	}
	else 
	{
		return ((CANDIDATELIST*)&m_candidate[0])->dwPageSize;
	}
}

/*----------------------------------------------------------------------------*/ 
int		TInput::GetCandidatePageStart() 
{
	if(m_candidate.empty()) 
	{
		return 0;
	}
	else 
	{
		return ((CANDIDATELIST*)&m_candidate[0])->dwPageStart;
	}
}

/*----------------------------------------------------------------------------*/ 
void	TInput::GetUnderLine(int* start, int* end) 
{ 	
	*start = WideCharToMultiByte(m_codePage, 0, m_comp.c_str(), m_ulStart, NULL, 0, NULL, NULL); 
	*end = WideCharToMultiByte(m_codePage, 0, m_comp.c_str(), m_ulEnd, NULL, 0, NULL, NULL); 
}

/*----------------------------------------------------------------------------*/ 
void	TInput::GetImeId()
{
	m_dwId[0] = m_dwId[1] = 0;

    if(!((m_hkl==_CHT_HKL) || (m_hkl==_CHT_HKL2) || (m_hkl==_CHS_HKL)))
		return;
        
	char imeFileName[256];
	if ( !ImmGetIMEFileNameA( m_hkl, imeFileName, ( sizeof(imeFileName) / sizeof(imeFileName[0]) ) - 1 ) )
        return;

    if ( !_GetReadingString ) 
	{
        if( ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHT_IMEFILENAME1, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHT_IMEFILENAME2, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHT_IMEFILENAME3, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHS_IMEFILENAME1, -1 ) != CSTR_EQUAL ) &&
            ( CompareStringA( LCID_INVARIANT, NORM_IGNORECASE, imeFileName, -1, CHS_IMEFILENAME2, -1 ) != CSTR_EQUAL ) )
		{
	        return;
        }
    }

    TCHAR temp[1024];
    DWORD dwVerHandle;
    DWORD dwVerSize = GetFileVersionInfoSize( temp, &dwVerHandle );

    if( dwVerSize ) 
	{
        LPVOID  lpVerBuffer = alloca( dwVerSize );

        if( GetFileVersionInfo( temp, dwVerHandle, dwVerSize, lpVerBuffer ) ) 
		{
			LPVOID  lpVerData;
			UINT    cbVerData;

            if( VerQueryValue( lpVerBuffer, _T("\\"), &lpVerData, &cbVerData ) ) 
			{
                DWORD dwVer = ( (VS_FIXEDFILEINFO*)lpVerData )->dwFileVersionMS;
                dwVer = ( dwVer & 0x00ff0000 ) << 8 | ( dwVer & 0x000000ff ) << 16;
                if( _GetReadingString
                    ||
                    ( m_langId == LANG_CHT &&
                        ( dwVer == MAKEIMEVERSION(4, 2) || 
                        dwVer == MAKEIMEVERSION(4, 3) || 
                        dwVer == MAKEIMEVERSION(4, 4) || 
                        dwVer == MAKEIMEVERSION(5, 0) ||
                        dwVer == MAKEIMEVERSION(5, 1) ||
                        dwVer == MAKEIMEVERSION(5, 2) ||
                        dwVer == MAKEIMEVERSION(6, 0) ) )
                    ||
                    ( m_langId == LANG_CHS &&
                        ( dwVer == MAKEIMEVERSION(4, 1) ||
                        dwVer == MAKEIMEVERSION(4, 2) ||
                        dwVer == MAKEIMEVERSION(5, 3) ) ) ) 
				{
                    m_dwId[0] = dwVer | m_langId;
                    m_dwId[1] = ( (VS_FIXEDFILEINFO*)lpVerData )->dwFileVersionLS;
                }
            }
        }
    }
}


/*----------------------------------------------------------------------------*/ 
void	TInput::SetupImeApi(HWND hWnd)
{
    char szImeFile[MAX_PATH + 1];

	_GetReadingString = NULL;
    _ShowReadingWindow = NULL;

	if( ImmGetIMEFileNameA( m_hkl, szImeFile, sizeof(szImeFile)/sizeof(szImeFile[0]) - 1 ) != 0 ) 
	{
		if( m_hDllIme ) FreeLibrary( m_hDllIme );
	    
		m_hDllIme = LoadLibraryA( szImeFile );

		if ( m_hDllIme ) 
		{
			_GetReadingString = (UINT (WINAPI*)(HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT))
				( GetProcAddress( m_hDllIme, "GetReadingString" ) );
			_ShowReadingWindow =(BOOL (WINAPI*)(HIMC, BOOL))
				( GetProcAddress( m_hDllIme, "ShowReadingWindow" ) );

			if( _ShowReadingWindow ) 
			{
				HIMC hImc = ImmGetContext(hWnd);
				if(hImc) 
				{
					_ShowReadingWindow( hImc, false );
					ImmReleaseContext(hWnd, hImc);
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------*/ 
bool	TInput::GetReadingWindowOrientation()
{
    bool bHorizontalReading = ( m_hkl == _CHS_HKL ) || ( m_hkl == _CHT_HKL2 ) || ( m_dwId[0] == 0 );
    if( !bHorizontalReading && ( m_dwId[0] & 0x0000FFFF ) == LANG_CHT )
    {
        char szRegPath[MAX_PATH];
        HKEY hKey;
        DWORD dwVer = m_dwId[0] & 0xFFFF0000;
        strcpy_s( szRegPath, "software\\microsoft\\windows\\currentversion\\" );
        strcat_s( szRegPath, ( dwVer >= MAKEIMEVERSION( 5, 1 ) ) ? "MSTCIPH" : "TINTLGNT" );
        LONG lRc = RegOpenKeyExA( HKEY_CURRENT_USER, szRegPath, 0, KEY_READ, &hKey );
        if (lRc == ERROR_SUCCESS)
        {
            DWORD dwSize = sizeof(DWORD), dwMapping, dwType;
            lRc = RegQueryValueExA( hKey, "Keyboard Mapping", NULL, &dwType, (PBYTE)&dwMapping, &dwSize );
            if (lRc == ERROR_SUCCESS)
            {
                if ( ( dwVer <= MAKEIMEVERSION( 5, 0 ) && 
                       ( (BYTE)dwMapping == 0x22 || (BYTE)dwMapping == 0x23 ) )
                     ||
                     ( ( dwVer == MAKEIMEVERSION( 5, 1 ) || dwVer == MAKEIMEVERSION( 5, 2 ) ) &&
                       (BYTE)dwMapping >= 0x22 && (BYTE)dwMapping <= 0x24 )
                   )
                {
                    bHorizontalReading = true;
                }
            }
            RegCloseKey( hKey );
        }
    }

	return bHorizontalReading;
}

/*----------------------------------------------------------------------------*/ 
void	TInput::GetPrivateReadingString(HWND hWnd)
{
    if( !m_dwId[0] ) 
	{
        m_reading.resize(0);
        return;
    }

	HIMC hImc = ImmGetContext(hWnd);
    if( !hImc ) 
	{
        m_reading.resize(0);
        return;
    }

    
    DWORD dwErr = 0;

    if( _GetReadingString ) 
	{
        UINT uMaxUiLen;
        BOOL bVertical;
        // Obtain the reading string size
        int wstrLen = _GetReadingString( hImc, 0, NULL, (PINT)&dwErr, &bVertical, &uMaxUiLen );

		if( wstrLen == 0 ) 
		{
			m_reading.resize(0);
		}
		else
		{
			wchar_t *wstr = (wchar_t*)alloca(sizeof(wchar_t) * wstrLen);
            _GetReadingString( hImc, wstrLen, wstr, (PINT)&dwErr, &bVertical, &uMaxUiLen );
			m_reading.assign(wstr, wstr+wstrLen);
		}

		m_bVerticalReading = bVertical ? true : false;

		ImmReleaseContext(hWnd, hImc);

    }
	else 
	{
        // IMEs that doesn't implement Reading String API
		wchar_t* temp;
		DWORD tempLen;
	    bool bUnicodeIme = false;
		INPUTCONTEXT *lpIC = g_imm.LockIMC(hImc);

		if(lpIC == NULL) 
		{
			temp = NULL;
			tempLen = 0;
		}
		else 
		{
			LPBYTE p = 0;
			switch( m_dwId[0] )
			{
				case IMEID_CHT_VER42: // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
				case IMEID_CHT_VER43: // New(Phonetic/ChanJie)IME98a : 4.3.x.x // WinMe, Win2k
				case IMEID_CHT_VER44: // New ChanJie IME98b          : 4.4.x.x // WinXP
					p = *(LPBYTE *)((LPBYTE)g_imm.LockIMCC( lpIC->hPrivate ) + 24 );
					if( !p ) break;
					tempLen = *(DWORD *)( p + 7 * 4 + 32 * 4 );
					dwErr = *(DWORD *)( p + 8 * 4 + 32 * 4 );
					temp = (wchar_t *)( p + 56 );
					bUnicodeIme = true;
					break;

				case IMEID_CHT_VER50: // 5.0.x.x // WinME
					p = *(LPBYTE *)( (LPBYTE)g_imm.LockIMCC( lpIC->hPrivate ) + 3 * 4 );
					if( !p ) break;
					p = *(LPBYTE *)( (LPBYTE)p + 1*4 + 5*4 + 4*2 );
					if( !p ) break;
					tempLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16);
					dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 + 1*4);
					temp = (wchar_t *)(p + 1*4 + (16*2+2*4) + 5*4);
					bUnicodeIme = false;
					break;

				case IMEID_CHT_VER51: // 5.1.x.x // IME2002(w/OfficeXP)
				case IMEID_CHT_VER52: // 5.2.x.x // (w/whistler)
				case IMEID_CHS_VER53: // 5.3.x.x // SCIME2k or MSPY3 (w/OfficeXP and Whistler)
					p = *(LPBYTE *)((LPBYTE)g_imm.LockIMCC( lpIC->hPrivate ) + 4);
					if( !p ) break;
					p = *(LPBYTE *)((LPBYTE)p + 1*4 + 5*4);
					if( !p ) break;
					tempLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2);
					dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2 + 1*4);
					temp  = (wchar_t *) (p + 1*4 + (16*2+2*4) + 5*4);
					bUnicodeIme = true;
					break;

				// the code tested only with Win 98 SE (MSPY 1.5/ ver 4.1.0.21)
				case IMEID_CHS_VER41:
					{
						int nOffset;
						nOffset = ( m_dwId[1] >= 0x00000002 ) ? 8 : 7;

						p = *(LPBYTE *)((LPBYTE)g_imm.LockIMCC( lpIC->hPrivate ) + nOffset * 4);
						if( !p ) break;
						tempLen = *(DWORD *)(p + 7*4 + 16*2*4);
						dwErr = *(DWORD *)(p + 8*4 + 16*2*4);
						dwErr = min( dwErr, tempLen );
						temp = (wchar_t *)(p + 6*4 + 16*2*1);
						bUnicodeIme = true;
					}
					break;

				case IMEID_CHS_VER42: // 4.2.x.x // SCIME98 or MSPY2 (w/Office2k, Win2k, WinME, etc)
					{
						OSVERSIONINFOA osi;
						osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
						GetVersionExA( &osi );

						int nTcharSize = ( osi.dwPlatformId == VER_PLATFORM_WIN32_NT ) ? sizeof(wchar_t) : sizeof(char);
						p = *(LPBYTE *)((LPBYTE)g_imm.LockIMCC( lpIC->hPrivate ) + 1*4 + 1*4 + 6*4);
						if( !p ) break;
						tempLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize);
						dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize + 1*4);
						temp  = (wchar_t *) (p + 1*4 + (16*2+2*4) + 5*4);
						bUnicodeIme = ( osi.dwPlatformId == VER_PLATFORM_WIN32_NT ) ? true : false;
					}
					break;

				default:
					temp = NULL;
					tempLen = 0;
					break;
			}
		}

		if(tempLen == 0) 
		{
			m_reading.resize(0);
		}
		else 
		{
			if( bUnicodeIme ) 
			{
				m_reading.assign(temp, tempLen);
			}
			else 
			{
				int wstrLen = MultiByteToWideChar(m_codePage, 0, (char*)temp, tempLen, NULL, 0); 
				wchar_t* wstr = (wchar_t*)alloca(sizeof(wchar_t)*wstrLen);
				MultiByteToWideChar(m_codePage, 0, (char*)temp, tempLen, wstr, wstrLen); 
				m_reading.assign(wstr, wstrLen);
			}
		}

		g_imm.UnlockIMCC(lpIC->hPrivate);
		g_imm.UnlockIMC(hImc);

		m_bVerticalReading = !GetReadingWindowOrientation();
    }

	ImmReleaseContext(hWnd, hImc);
}

/*----------------------------------------------------------------------------*/ 
void	TInput::CheckToggleState(HWND hWnd)
{
	/* Update Indicator */ 
    switch (PRIMARYLANGID(m_langId)) 
	{
    case LANG_KOREAN:
        m_bVerticalCandidate = false;
        m_wszCurrIndicator = g_aszIndicator[INDICATOR_KOREAN];
        break;

    case LANG_JAPANESE:
        m_bVerticalCandidate = true;
        m_wszCurrIndicator = g_aszIndicator[INDICATOR_JAPANESE];
        break;

    case LANG_CHINESE:

	    m_bVerticalCandidate = true;
		switch(SUBLANGID(m_langId)) 
		{
        case SUBLANG_CHINESE_SIMPLIFIED:
            m_bVerticalCandidate = m_dwId[0] == 0;
            m_wszCurrIndicator = g_aszIndicator[INDICATOR_CHS];
            break;

        case SUBLANG_CHINESE_TRADITIONAL:
            m_wszCurrIndicator = g_aszIndicator[INDICATOR_CHT];
            break;

        default:
            m_wszCurrIndicator = g_aszIndicator[INDICATOR_NON_IME];
            break;
        }
        break;

    default:
        m_wszCurrIndicator = g_aszIndicator[INDICATOR_NON_IME];
		break;
    }

    if( m_wszCurrIndicator == g_aszIndicator[INDICATOR_NON_IME] )
    {
        char szLang[10];
        GetLocaleInfoA( MAKELCID( m_langId, SORT_DEFAULT ), LOCALE_SABBREVLANGNAME, szLang, sizeof(szLang) );
        m_wszCurrIndicator[0] = szLang[0];
        m_wszCurrIndicator[1] = towlower( szLang[1] );
    }

	
	
	/* Check Toggle State */ 
	bool bIme = ImmIsIME( m_hkl ) != 0;

	HIMC hImc = ImmGetContext(hWnd);
    if( hImc ) 
	{
        if( ( PRIMARYLANGID(m_langId) == LANG_CHINESE ) && bIme ) 
		{
            DWORD dwConvMode, dwSentMode;
			ImmGetConversionStatus(hImc, &dwConvMode, &dwSentMode);
            m_ImeState = ( dwConvMode & IME_CMODE_NATIVE ) ? IMEUI_STATE_ON : IMEUI_STATE_ENGLISH;

        }
		else 
		{
            m_ImeState = ( bIme && ImmGetOpenStatus(hImc) != 0 ) ? IMEUI_STATE_ON : IMEUI_STATE_OFF;
        }
		ImmReleaseContext(hWnd, hImc);
        
	}
	else 
	{

        m_ImeState = IMEUI_STATE_OFF;
	}
}

/*----------------------------------------------------------------------------*/ 
int	GetCharsetFromLang( LANGID langid )
{
	switch( PRIMARYLANGID(langid) )
	{
	case LANG_JAPANESE:
		return SHIFTJIS_CHARSET;
	case LANG_KOREAN:
		return HANGEUL_CHARSET;
	case LANG_CHINESE:
		switch( SUBLANGID(langid) )
		{
		case SUBLANG_CHINESE_SIMPLIFIED:
			return GB2312_CHARSET;
		case SUBLANG_CHINESE_TRADITIONAL:
			return CHINESEBIG5_CHARSET;
		default:
			return ANSI_CHARSET;
		}
	case LANG_GREEK:
		return GREEK_CHARSET;
	case LANG_TURKISH:
		return TURKISH_CHARSET;
	case LANG_HEBREW:
		return HEBREW_CHARSET;
	case LANG_ARABIC:
		return ARABIC_CHARSET;
	case LANG_ESTONIAN:
	case LANG_LATVIAN:
	case LANG_LITHUANIAN:
		return BALTIC_CHARSET;
	case LANG_THAI:
		return THAI_CHARSET;
	case LANG_CZECH:
	case LANG_HUNGARIAN:
	case LANG_POLISH:
	case LANG_CROATIAN:
	case LANG_MACEDONIAN:
	case LANG_ROMANIAN:
	case LANG_SLOVAK:
	case LANG_SLOVENIAN:
		return EASTEUROPE_CHARSET;
	case LANG_RUSSIAN:
	case LANG_BELARUSIAN:
	case LANG_BULGARIAN:
	case LANG_UKRAINIAN:
		return RUSSIAN_CHARSET;
	case LANG_VIETNAMESE:
		return VIETNAMESE_CHARSET;
	default:
		return ANSI_CHARSET;
	}
}

/*----------------------------------------------------------------------------*/ 
int	GetCodePageFromCharset( int charset )
{
	switch( charset )
	{
	case SHIFTJIS_CHARSET:
		return 932;
	case HANGUL_CHARSET:
		return 949;
	case GB2312_CHARSET:
		return 936;
	case CHINESEBIG5_CHARSET:
		return 950;
	case GREEK_CHARSET:
		return 1253;
	case TURKISH_CHARSET:
		return 1254;
	case HEBREW_CHARSET:
		return 1255;
	case ARABIC_CHARSET:
		return 1256;
	case BALTIC_CHARSET:
		return 1257;
	case THAI_CHARSET:
		return 874;
	case EASTEUROPE_CHARSET:
		return 1250;
	case VIETNAMESE_CHARSET:
		return 1258;
	default:
		return 1252;
	}
}

/*----------------------------------------------------------------------------*/ 
int	GetCodePageFromLang( LANGID langid )
{
	return GetCodePageFromCharset(GetCharsetFromLang(langid));
}

/*----------------------------------------------------------------------------*/ 
