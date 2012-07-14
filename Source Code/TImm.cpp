// CImm.cpp - part if Input Context management class
//
// Copyright (C) 2000-2005, Kwon-il Lee
//
// Kwon-il Lee
// zupet@hitel.net


#include "stdafx.h"
#include "TImm.h"


/*----------------------------------------------------------------------------*/ /* TImm */ 
TImm::TImm() 
:	m_hDllImm32(NULL) 
{
}

/*----------------------------------------------------------------------------*/ 
TImm::~TImm()
{ 
	if(m_hDllImm32) 
		FreeLibrary(m_hDllImm32);
}

/*----------------------------------------------------------------------------*/ 
void	TImm::Init()
{
    CHAR szPath[MAX_PATH+1];
    
	if( GetSystemDirectoryA( szPath, MAX_PATH+1 ) ) 
	{
		strcat_s( szPath, "\\imm32.dll" );
		m_hDllImm32 = LoadLibraryA( szPath );
		if( m_hDllImm32 )
		{
			_ImmLockIMC		= (INPUTCONTEXT*(WINAPI *)( HIMC ))
				GetProcAddress( m_hDllImm32, "ImmLockIMC" );
			_ImmUnlockIMC	= (BOOL(WINAPI *)( HIMC ))
				GetProcAddress( m_hDllImm32, "ImmUnlockIMC" );
			_ImmLockIMCC	= (LPVOID(WINAPI *)( HIMCC ))
				GetProcAddress( m_hDllImm32, "ImmLockIMCC" );
			_ImmUnlockIMCC	= (BOOL(WINAPI *)( HIMCC ))
				GetProcAddress( m_hDllImm32, "ImmUnlockIMCC" );
		}
	}
}

/*----------------------------------------------------------------------------*/ 
INPUTCONTEXT*	TImm::LockIMC(HIMC hImc)
{
	if(_ImmLockIMC == NULL) 
	{
		return NULL;
	}
	else 
	{
		return _ImmLockIMC(hImc);
	}
}

/*----------------------------------------------------------------------------*/ 
BOOL	TImm::UnlockIMC(HIMC hImc)
{
	if(_ImmUnlockIMC == NULL) 
	{
		return FALSE;
	}
	else 
	{
		return _ImmUnlockIMC(hImc);
	}
}

/*----------------------------------------------------------------------------*/ 
LPVOID	TImm::LockIMCC( HIMCC himcc )
{
	if(_ImmLockIMCC == NULL) 
	{
		return NULL;
	}
	else 
	{
		return _ImmLockIMCC(himcc);
	}
}

/*----------------------------------------------------------------------------*/ 
BOOL	TImm::UnlockIMCC( HIMCC himcc )
{
	if(_ImmUnlockIMCC == NULL) 
	{
		return NULL;
	}
	else 
	{
		return _ImmUnlockIMCC(himcc);
	}
}

/*----------------------------------------------------------------------------*/ 

