#include <utils/input.h>

#ifdef __WIN32__
	#include <winsock2.h>
#else
	#include <termios.h>
#endif

using namespace std;

void Utils::SetStdinEcho(bool enable)
{
#ifdef __WIN32__
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if (!enable)
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode);
#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);

    if (!enable)
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

string Utils::get_file_contents(ifstream& input)
{
	string contents;

	if (input)
	{
		input.seekg(0, ios::end);
		contents.resize(input.tellg());
		input.seekg(0, ios::beg);
		input.read(&contents[0], contents.size());
		input.close();
	}

	return contents;
}
