// CImm.h - part if Input Context management class
//
// Copyright (C) 2000-2005, Kwon-il Lee
//
// Kwon-il Lee
// zupet@hitel.net


#if !defined(_C_IME_HANDLER_H_)
#define _C_IME_HANDLER_H_

class TImm
{
public:
	TImm();
	virtual ~TImm();

	void	Init();

	INPUTCONTEXT*	LockIMC(HIMC hImc);
	BOOL			UnlockIMC(HIMC hImc);
	LPVOID			LockIMCC( HIMCC );
	BOOL			UnlockIMCC( HIMCC );

protected:

	HINSTANCE		m_hDllImm32;

	INPUTCONTEXT*	(WINAPI * _ImmLockIMC)( HIMC );
	BOOL			(WINAPI * _ImmUnlockIMC)( HIMC );
	LPVOID			(WINAPI * _ImmLockIMCC)( HIMCC );
	BOOL			(WINAPI * _ImmUnlockIMCC)( HIMCC );
};

#endif //_C_IME_HANDLER_H_