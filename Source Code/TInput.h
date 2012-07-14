// TInput.h - part if Multi-Language input class
//
// Copyright (C) 2000-2005, Kwon-il Lee
//
// Kwon-il Lee
// zupet@hitel.net


#if !defined(_C_INPUT_H_)
#define _C_INPUT_H_


/*----------------------------------------------------------------------------*/ 
int GetCharsetFromLang(LANGID);
int	GetCodePageFromLang( LANGID langid );

/*----------------------------------------------------------------------------*/ 
class TInput
{
public:
	TInput();
	~TInput();

	void		InitInput();

	/* messages */ 
	void		OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void		OnInputLanguageChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool		OnComposition(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool		OnEndComposition(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool		OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool		OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam);

	int			GetInput(wchar_t* text, int len);
	int			GetComp(wchar_t* text, int len);
	int			GetReading(wchar_t* text, int len);
	int			GetCandidate(DWORD index, wchar_t* text, int len);

	int			GetInput(char* text, int len);
	int			GetComp(char* text, int len);
	int			GetReading(char* text, int len);
	int			GetCandidate(DWORD index, char* text, int len);

	int			GetCandidateCount();
	int			GetCandidateSelection();
	int			GetCandidatePageSize();
	int			GetCandidatePageStart();
	void		GetUnderLine(int* start, int* end);

	/* property */ 
	bool		IsVerticalReading(){ return m_bVerticalReading; }
	bool		IsVerticalCandidate(){ return m_bVerticalCandidate; }
	WORD		GetCodePage() { return m_codePage; }
	LANGID		GetLangId() { return m_langId; }
	int			GetCharSet() { return GetCharsetFromLang(m_langId); }
	wchar_t*	GetIndicator() { return m_wszCurrIndicator; }
	int			GetImeState() { return m_ImeState; }


protected:
	void		SetupImeApi(HWND hWnd);
	void		GetImeId();
	bool		GetReadingWindowOrientation();
	void		GetPrivateReadingString(HWND hWnd);
	void		CheckToggleState(HWND hWnd);

protected:
	std::wstring		m_input;
	std::wstring		m_comp;
	std::wstring		m_reading;
	std::vector<BYTE>	m_candidate;

	int					m_ulStart;
	int					m_ulEnd;

	HKL					m_hkl;
	LANGID				m_langId;
	WORD				m_codePage;

	bool				m_bUnicodeIME;
	bool				m_bVerticalReading;
	bool				m_bVerticalCandidate;
	int					m_ImeState;
	wchar_t*			m_wszCurrIndicator;

	DWORD				m_dwId[2];

	HINSTANCE			m_hDllIme;

	ITfThreadMgr*		m_ptim;

	UINT				(WINAPI * _GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT );
	BOOL				(WINAPI * _ShowReadingWindow)( HIMC, BOOL );
};

/*----------------------------------------------------------------------------*/ 


#endif //_C_INPUT_H_