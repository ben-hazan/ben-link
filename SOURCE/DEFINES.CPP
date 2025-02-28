#ifndef __DEFINES_CPP
#define __DEFINES_CPP

// THIS file mainly holds the definitions of the project.

#define MAX_FILES_IN_DIR 300
// maximum files which can be read.


#define MAX_WAIT 25
// maximum time to loop for reading from port.

#define MAXCOMMAND 80
// maximum characters to read from port in server mode.

// MAXPATH = 80
// is defined in dir.h

#ifndef OK
#define OK 0
#endif
// OK definition. may be used in some functions. mainly 0 is OK usually.

#define FILE_WINS_LINES 21
#define FILE_WINS_WIDTH 36
// definitions of the filelists window size.

#define FROMUP 1
#define FROMDOWN 1
// how many rows from above and below the filelists to keep for
// the filelist head and bottom.

#define T_BLOCK_SIZE 512
// transfer block size in characters.

#define BUTTONSIZE 10
// The maximum characters for a button ('overwrite' is the biggest button).

#define NUMOFMENUS 6
// number of pulldown menus.

#define MENUSIZE 20
// width and meximum letters available in the pulldown menu line.

#define MENUEND 100
#define MENULEFT 101
#define MENURIGHT 102
// definitions for returned menu commands.

#endif