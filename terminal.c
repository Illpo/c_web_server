#include <windows.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>

int t_start() {
    SetConsoleTitleW(L"  ▀▄▀  ▀▄▀  ");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int result = _setmode( _fileno( stdout ), _O_U8TEXT );
    
    if( result == -1 ) {
      perror( "Cannot setmode" );
    }

    wprintf(L"┌──Server running...\n└─» HELLO ");
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    wprintf(L"Mr. ILLPO \n");
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    wprintf(L" ┌───┐  ┌───┐\n | ▀▄▀  ▀▄▀ |\n └───┘  └───┘\n");


    if (_setmode(_fileno( stdout ), result) == -1) {
        perror("_setmode");
        return 1;
    }

    return 1;
}