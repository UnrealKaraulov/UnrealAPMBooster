#include <Windows.h>
#include "verinfo.h"
#include <time.h>

#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0

int * IsInGame = 0;
int * IsInGame2 = 0;
int GameDll = 0;
int _W3XGlobalClass = 0;
int CMD_NO_TARGET_ADDR = 0;

BOOL IsGame( )
{
	return *IsInGame2 > 0 || *IsInGame > 0;
}

BOOL NeedStop = TRUE;

#define ADDR(X,REG)\
	__asm MOV REG, DWORD PTR DS : [ X ] \
	__asm MOV REG, DWORD PTR DS : [ REG ]

int PressCommandWithoutTarget( int a1, int a2, unsigned int a3, unsigned int a4 )
{
	if ( *( int* ) _W3XGlobalClass > 0 )
	{
		__asm
		{
			PUSH a4;
			PUSH a3;
			ADDR( _W3XGlobalClass, ECX );
			MOV ECX, DWORD PTR DS : [ ECX + 0x1B4 ];
			PUSH a2;
			PUSH a1;
			CALL CMD_NO_TARGET_ADDR;
		}
	}
	return 0;
}


void PressStop( )
{
	PressCommandWithoutTarget( 0xd0004, 0, 1, 0 );
}

void PressHold( )
{
	PressCommandWithoutTarget( 0xd0019, 0, 1, 0 );
}

void PressStopOrHold( )
{
	NeedStop = !NeedStop;

	if ( NeedStop )
	{
		PressStop( );
	}
	else
	{
		PressHold( );
	}
}

void TimerWorker( )
{
	if ( IsKeyPressed( VK_LCONTROL ) && IsKeyPressed( 0x42 ) )
	{
		PressStopOrHold( );
	}
}


VOID CALLBACK MySuperTimer(
	HWND hwnd,        // handle to window for timer messages 
	UINT message,     // WM_TIMER message 
	UINT idTimer,     // timer identifier 
	DWORD dwTime )     // current system time 
{
	if ( IsGame( ) )
		TimerWorker( );
}

HWND War3HWND = NULL;
UINT_PTR TimerID = NULL;
UINT_PTR ttt = NULL;

BOOL __stdcall DllMain( HINSTANCE i, DWORD r, LPVOID )
{
	if ( r == DLL_PROCESS_ATTACH )
	{
		HMODULE hGameDll = GetModuleHandle( "Game.dll" );
		GameDll = ( int ) hGameDll;
		if ( !hGameDll )
		{
			MessageBox( 0, "No game dll found!", "ERROR IN APM BOOSTER", 0 );
			return FALSE;
		}

		CFileVersionInfo gdllver;
		gdllver.Open( hGameDll );
		// Game.dll version (1.XX)
		int GameDllVer = gdllver.GetFileVersionQFE( );
		gdllver.Close( );

		if ( GameDllVer == 6401 )
		{
			IsInGame = ( int* ) ( GameDll + 0xAB62A4 );
			IsInGame2 = ( int* ) ( GameDll + 0xACF678 );
			CMD_NO_TARGET_ADDR = GameDll + 0x339C60;
			_W3XGlobalClass = GameDll + 0xAB4F80;
		}
		else if ( GameDllVer == 52240 )
		{
			IsInGame = ( int* ) ( GameDll + 0xBE6530 );
			IsInGame2 = ( int* ) ( GameDll + 0xBE6530 );
			CMD_NO_TARGET_ADDR = GameDll + 0x3AE4E0;
			_W3XGlobalClass = GameDll + 0xBE6350;
		}
		else
		{
			MessageBox( 0, "APM BOOSTER: Game version not supported!", "ERROR IN APM BOOSTER", 0 );
			return FALSE;
		}

		srand( ( unsigned int ) time( NULL ) );

		War3HWND = FindWindow( "Warcraft III", NULL );
		if ( War3HWND == NULL )
			War3HWND = FindWindow( NULL, "Warcraft III" );
		TimerID = 800 + ( rand( ) % 50 );

		ttt = SetTimer( War3HWND, TimerID, 20, MySuperTimer );


	}
	else if ( r == DLL_PROCESS_DETACH )
	{
		if ( ttt )
			KillTimer( War3HWND, TimerID );
	}

	return TRUE;
}