/*-	Keyboard's scancodes	-*/
#ifndef __SCANCODE_CPP
#define __SCANCODE_CPP

#ifndef __CONIO_H
#include <conio.h>
#endif

#ifndef __STRING_H
#include <string.h>
#endif

// definitions of the keystrokes.

#define ESC 0x1b
#define F1 0xbb
#define F2 0xbc
#define F3 0xbd
#define F4 0xbe
#define F5 0xbf
#define F6 0xc0
#define F7 0xc1
#define F8 0xc2
#define F9 0xc3
#define F0 0xc4
#define BACKSPACE 0x08
#define HOME 0xc7
#define END 0xcF
#define INS 0xd2
#define DEL 0xd3
#define TAB 0x09
#define SPACE 0x20
#define UP 0xc8
#define DOWN 0xd0
#define LEFT 0xcb
#define RIGHT 0xcd
#define PGUP 0xc9
#define PGDN 0xd1
#define ENTER 0x0d

// gets a pressed key. 0 not a key is pressed.
unsigned char getkey()
{
char k;
	if (kbhit()) {
		k = getch();
		if (!k) {
			k = getch();
			k+=0x80;
		}
		return k;
	} else {
		return 0;
	}
}

// waits for a key press then returns the stroke.
unsigned char waitkey()
{
unsigned char k;
	k = getch();
	if (!k) {
		k = getch();
		k +=0x80;
	}
	return k;
}

// gets a whole string (Allows editing!).
char getstring(char *s,int max)
{
	int c=0;
	int l;
	s[l]=NULL;
	int i;
	unsigned char k;
	int x = wherex(),y = wherey();

	for (i = 0 ; i < max ; i++)cprintf("ù");
	gotoxy(wherex()-max,y);
	_setcursortype(_NORMALCURSOR);

	l = strlen(s);

	s[l]=NULL;
	gotoxy(x,y);
	cprintf("%s",s);
	for (i = l ; i < max ; i++)cprintf("ù");
	gotoxy(x+c,y);

	do {
		k = waitkey();

		if (k==ESC){
			 s[0]=NULL;
			 _setcursortype(_NOCURSOR);
			 return -1;
		}else if (k==TAB) {
			 s[l]=NULL;
			 _setcursortype(_NOCURSOR);
			 return -2;
		}else if (k==F0) {
			 s[l]=NULL;
			 _setcursortype(_NOCURSOR);
			 return -3;
		}else if (k==RIGHT) {
			if (c<l) {
				c++;
				gotoxy(wherex()+1,y);
			}
		}else if (k==LEFT) {
			if (c>0) {
				c--;
				gotoxy(wherex()-1,y);
			}
		}else if (k==SPACE) {
			if (l<max) {
				for (i = l-1 ; i >= c ; i--) s[i+1]=s[i];
				s[c]='_';
				l++;c++;
				s[l]=NULL;
				gotoxy(x,y);
				cprintf("%s",s);
				for (i = l ; i < max ; i++)cprintf("ù");
				gotoxy(x+c,y);
			}
		}else if (k==DEL) {
			if (c<l) {
				l--;
				for (i = c ; i < l ; i++) s[i]=s[i+1];
				gotoxy(x,y);
				s[l]=NULL;
				cprintf("%s",s);
				for (i = l ; i < max ; i++)cprintf("ù");
				gotoxy(x+c,y);
			}
		}else if (k==BACKSPACE) {
			if (c>0) {
				c--;
				for (i = c ; i < l ; i++) s[i]=s[i+1];
				l--;
				gotoxy(x,y);
				s[l]=NULL;
				cprintf("%s",s);
				for (i = l ; i < max ; i++)cprintf("ù");
				gotoxy(x+c,y);
			}
		}else if (k==HOME) {
			c=0;
			gotoxy(x,y);
		}else if (k==END) {
			c=l;
			gotoxy(x+c,y);
		} else {
			if (l<max) {
				if (((k>='a')&&(k<='z'))||((k>='A')&&(k<='Z'))||((k>='0')&&(k<='9'))||(k=='.')||(k=='~')||(k=='-')||(k=='\\')||((k>='!')&&(k<=')'))||(k==':')) {
					for (i = l-1 ; i >= c ; i--) s[i+1]=s[i];
					s[c]=k;
					l++;c++;
					s[l]=NULL;
					gotoxy(x,y);
					cprintf("%s",s);
					for (i = l ; i < max ; i++)cprintf("ù");
					gotoxy(x+c,y);
				}
			}
		}

	} while (k!=ENTER);
	_setcursortype(_NOCURSOR);
	return l;
}
#endif