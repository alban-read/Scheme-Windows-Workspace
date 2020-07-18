// added this to schsig.c 
// traps escape and turns it into the keyboard interrupt.
// this is then caught in base.ss and turned into an error.

DWORD WINAPI escape_watcher(LPVOID x)
{
	while (TRUE) {
		
		Sleep(1);
		if (GetAsyncKeyState(VK_ESCAPE) != 0)  
		{	 
			  tc_mutex_acquire()
			  ptr ls;
			   for (ls = S_threads; ls != Snil; ls = Scdr(ls)) {
				 ptr tc=THREADTC(Scar(ls));
				 KEYBOARDINTERRUPTPENDING(tc) = Strue;
			   } 
			   tc_mutex_release()
		}
		Sleep(125);
 
	}		
}
static void init_signal_handlers() {
	
    HANDLE escape_key_thread = CreateThread(
			0,
			0,
			escape_watcher,
			0,
			0,
			0);
}