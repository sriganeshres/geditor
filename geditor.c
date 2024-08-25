#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios originaltermios;

void die(const char *s)
{
    perror(s);
    exit(1);
}

void disableRaw()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &originaltermios) == -1)
        die("tcsetattr");
}
void enableRaw()
{
    tcgetattr(STDIN_FILENO, &originaltermios);
    atexit(disableRaw); // defer in go routine.

    struct termios raw = originaltermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    /*
        c_lflag = local_flag
        c_iflag = input_flag
        c_oflag = output_flag
        c_cflag = control_flag
        ECHO to disable printing everything back to console.
        ICANON for reading byte by byte instead of line by line.
        ISIG to disable SIGINT (CTRL+C) and SIGTSTP (CTRL+Z) handling.
        IXON and IXOFF are to control transmission of I/O.
        IEXTEN to disable Ctrl-V.
        ICRNL to disable Ctrl-M. CR = Carraige Return NL = new line.
        When BRKINT is turned on, a break condition will cause a SIGINT signal to be sent to the program, like pressing Ctrl-C.
        INPCK enables parity checking, which doesn’t seem to apply to modern terminal emulators.
        ISTRIP causes the 8th bit of each input byte to be stripped, meaning it will set it to 0. This is probably already turned off.
        CS8 is not a flag, it is a bit mask with multiple bits, which we set using the bitwise-OR (|) operator unlike all the flags we are turning off. It sets the character size (CS) to 8 bits per byte. On my system, it’s already set that way.
        c_cc for timeouts
    */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
    enableRaw();
    while (1)
    {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        if (iscntrl(c))
        {
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q')
            break;
    }
    return 0;
}