/*************************/
/**  Display managment  **/
/*************************/
#ifndef __TEXTWIN_CPP
#define __TEXTWIN_CPP

#ifndef __DEFINES_CPP
#include <defines.cpp>
#endif
#ifndef __CONIO_H
#include <conio.h>
#endif
#ifndef __DOS_H
#include <dos.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif
#ifndef __SCANCODE_CPP
#include <scancode.cpp>
#endif
#ifndef __CONFIG_CPP
#include <config.cpp>
#endif

class textwin
{
	int placex,placey;   // where is the window.
	int sizex,sizey;     // size of the window.
	char color;	     // color of windows text.
	char bordercolor;   // colors of the border.
	char *background;   // the screen under the window buffer.
	struct text_info before; // text mode before the window.
public:
	void show(char mask); // shows the window and defines it (using conio.h->window())
	void erase();	      // deletes the window.
	char *headline;	      // headline.
	// constructor.
	textwin(int x,int y,int sx,int sy,char c,char bc,char *h) {
		gettextinfo(&before);
		placex = x;
		placey = y;
		sizex = sx;
		sizey = sy;
		color = c;
		bordercolor = bc;
		headline = h;
		background = new char[(sizex+2)*(sizey+2)*2];
		gettext(placex,placey,placex+sizex+1,placey+sizey+1,background);
	}

	// destructor.
	~textwin() {
		erase();
		window(before.winleft,before.wintop,before.winright,before.winbottom);
		textattr(before.attribute);
		gotoxy(before.curx,before.cury);
		delete background;
	}
};



/*- Shows a window -*/
void textwin::show(char mask)
{
	int i;
	window(1,1,80,25);
	if (mask) textattr(bordercolor|0x08);
	else textattr(bordercolor&0xf7);
	gotoxy(placex,placey);
	cprintf("%c",201);

	for (i = 0 ; i < sizex ; i++)
		cprintf("%c",205);

	cprintf("%c",187);
	gotoxy(placex+2,placey);
	strxfrm(headline,headline,sizex-2);
	cprintf("[%s]",headline);
	for (i = 0 ; i < sizey ; i++)
		{
		gotoxy(placex,placey+1+i);
		cprintf("%c",186);
		}

	for (i = 0 ; i < sizey ; i++)
		{
		gotoxy(placex+sizex+1,placey+1+i);
		cprintf("%c",186);
		}
	gotoxy(placex,placey+sizey+1);
	cprintf("%c",200);

	for (i = 0 ; i < sizex ; i++)
		{
		cprintf("%c",205);
		}
	cprintf("%c",188);
	window(placex+1,placey+1,placex+sizex,placey+sizey);
	textattr(color);
	clrscr();
}

void textwin::erase()
{
	puttext(placex,placey,placex+sizex+1,placey+sizey+1,background);
}

/*-------- Screen functions not related to windows. ------------*/

// prints the bottom line on screen. (hotkeys summary).
void print_hot_keys(char attr,char boldattr)
{
	int i;
	const char hotkey[][5]={
		"ChDr", //1
		"Srvr", //2
		"LogX", //3
		"Edit", //4
		"Copy", //5
		"Move", //6
		"NewD", //7
		"Delt", //8
		"Menu", //9
		"Quit" //10
	};

	gotoxy(1,24);
	textattr(attr);
	for (i = 0 ; i < 10 ; i++) {
		textattr(boldattr);
		cprintf("F%-2d",i+1);
		textattr(attr);
		cprintf("%-4s ",hotkey[i]);
	}
	gotoxy(1,24);
	textattr(0x07);
	insline();
}

// clears the bottom line.
void clear_hot_keys()
{
	gotoxy(1,25);
	insline();
}

// prints a button.
void button(char *str,char curattr,char attr,int lighted)
{
	if (lighted) attr=attr|0x88;
	else	     attr=attr&0x77;

	cprintf(" ");
	textattr(attr);
	cprintf("%c%s%c",17,str,16);
	textattr(curattr);
	cprintf(" ");
}

// prompts for a button. return the button num in list.
int choosebutton(int argn, char str[][BUTTONSIZE],char curattr, char attr)
{
	int i;
	int x = wherex(),y = wherey();
	int cursor =0;
	unsigned char k;

	while(getkey()); // empty keyboard buffer so user won't enter shit
			 // he doesn't want to enter.
	button(str[0],curattr,attr,1);

	for (i = 1 ; i < argn ; i++)
		button(str[i],curattr,attr,0);

	do {
		k = waitkey();

		if ((k == LEFT)||(k == UP)) {
			if (cursor > 0) {
				cursor--;
			} else {
				cursor=argn-1;
			}
			gotoxy(x,y);
			for (i = 0 ; i < argn ; i++)
				button(str[i],curattr,attr,(i==cursor));
		} else
		if ((k == RIGHT)||(k == DOWN)||(k == TAB)) {
			if (cursor < argn-1) {
				cursor++;
			} else {
				cursor=0;
			}
			gotoxy(x,y);
			for (i = 0 ; i < argn ; i++)
				button(str[i],curattr,attr,(i==cursor));
		}
		else if (k == ESC) return -1;
		else if (k == F0) return -2;
		else if ((k>='a')&&(k<='z')) {
			k-='a';k+='A';
		}
		if ((k>='A')&&(k<='Z')) {
			for (i = 0 ; i < argn ; i++)
				if (str[i][0]==k)
					{cursor=i;break;}
			gotoxy(x,y);
			for (i = 0 ; i < argn ; i++)
				button(str[i],curattr,attr,(i==cursor));

		}
	} while (k != ENTER);
	return cursor;
}


// called usually on error.
// opens a window, prints a msg and sounds a beep.
void msgbox(char*h,char*s)
{
	char button[][BUTTONSIZE] = {
		"OK"};
	textwin mbox(5,11,70,2,0x4c,0x4f,h);
	mbox.show(1);
	cprintf("%s\n\r                                ",s);
	if (SOUND)cprintf("\a");
	choosebutton(1,button,0x4f,0x5f);
}

#endif