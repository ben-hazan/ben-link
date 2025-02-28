/********************/
/**  Data handling **/
/********************/
#ifndef __FILELIST_CPP
#define __FILELIST_CPP

#ifndef __CONIO_H
#include <conio.h>
#endif
#ifndef __DIR_H
#include <dir.h>
#endif
#ifndef __DOS_H
#include <dos.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif


#define FROMUP 1
#define FROMDOWN 1

class filelist {
	int rows,cols;		// rows and cols in the list.
	int page;       	// page number
	int max;		// maximum files
	int cursor;		// cursor position.
	char col,bcol;		// colors of list.
public:
	int num;		// num of files in list
	struct ffblk*list;	// list itself
	char *taglist;		// char =1 means tagged file.
	char path[MAXPATH];	// working directory of list.
	int drive;		// working drive of list.
	int farcomp;		// watch for FAR LISTS!

	filelist(int n,int m,int cur,char c,char bc,int ro,int co);
			     // constructor.
	~filelist()
		{
		clrscr();
		delete list;                // destructor.
		delete taglist;
		}

	void movepos(int howmuch);    // moves the cursor.
	void home();			// sets cursor to pos 0.
	void end();                   // sets cursor to pos num (which is the number of files).
	int returnpos(); 		// returns the cursor position.
	void removecursor();		// hides the cursor/ removes it from screen.

	void updatebottom();	       // updates the tag information.
	void showrow(int n);		// prints one.
	void show();			// shows a page from the list.


	int tagnum;			// number of tagged files.
	long tagsize;			// tot size of tagged files.
	void logicalnottag();		// logical not the tag array.
	void tagall();			// tags all files (note: ".." does not tag).
	void zerotag();			// zero all tags.
	void tag();			// tag cursord file. move to the next file (note: ".." does not tag).

	int sortby;			// 0:just by directory.
					// 1:by name.
					// 2:by size. (directory's still by name).
					// 3:by date.
					// 4:by time.
	void sort();			// ZORT! (from Pinky and the Brain you fool!).
	void swaprecord(int s,int d);	// uses sort to swap the ffblk (of list) and char (of taglist)
};

filelist::filelist(int n,int m,int cur,char c,char bc,int ro,int co)
{
	num=n;
	max=m;
	cursor=cur;
	list=new struct ffblk[max];   // aloc memory.
	taglist=new char[max];
	page=0;
	col=c&0xf7;     // untagged colors -> unlighted.
	bcol=bc&0xf7;
	rows=ro;
	cols=co;
	tagnum=0;
	tagsize=0;
	sortby=1;	// initially sort by name.
	farcomp=0;
}


void filelist::zerotag()             // zeros the tags. usually called on
{                                    // reread of filelist.
	tagnum=0;
	tagsize=0;
	for (int i = 0 ; i < max ; i++)
		taglist[i]=0;
}


void filelist::showrow(int n)        // shows one row from the filelist.
{
	cprintf("%-12s",list[n].ff_name);

	if (cols>=1) {
		if (list[n].ff_attrib==FA_DIREC)
			cprintf("³%-9s","<SUB-DIR>");
		else
			cprintf("³%9ld",list[n].ff_fsize);
	}
	if (cols>=2) {
		cprintf("³%2d.%02d.%2d",(list[n].ff_fdate) & 0x1f,(list[n].ff_fdate >> 5 ) & 0x0f,((list[n].ff_fdate >> 9)+80)%100);
	}
	if (cols>=3) {
		cprintf("³%02d:%02d",(list[n].ff_ftime>>11),(list[n].ff_ftime >> 5) & 0x3f);
	}

	if (n-page < rows-1) cprintf("\n\r");

}
				    // shows entire filelist. including cursor.
void filelist::show()
{
	int i;
	textattr(col);
	clrscr();
	gotoxy(1,1);
	i = 0;

	textattr(0x1e);

	if (sortby==1) textattr(0x1a);
	cprintf(" Name       ");
	if (sortby==1) textattr(0x1e);

	if (cols>=1) {
		cprintf("³");
		if (sortby==2) textattr(0x1a);
		cprintf(" Size    ");
		if (sortby==2) textattr(0x1e);
	}

	if (cols>=2) {
		cprintf("³");
		if (sortby==3) textattr(0x1a);
		cprintf(" Date   ");
		if (sortby==3) textattr(0x1e);
	}

	if (cols>=3) {
		cprintf("³");
		if (sortby==4) textattr(0x1a);
		cprintf("Time ");
		if (sortby==4) textattr(0x1a);
	}

	cprintf("\n\r");

	while ((i < rows)&&(i+page < num)) {
		if (i==cursor)textattr(bcol|((taglist[i+page])?8:0));
		else	      textattr( col|((taglist[i+page])?8:0));
		showrow(i+page);
		i++;
	}
	textattr(0x1e);
	updatebottom();
}

void filelist::removecursor()     // removes the cursor. called when u change filelist.
{
	if (num != 0) {
		gotoxy(1,cursor+1+FROMUP);
		textattr(col|((taglist[page+cursor])?8:0));
		showrow(page+cursor);
	}
}


// moves the position of the cursor in howmuch steps.
// does echo to screen. does flipping.
void filelist::movepos(int howmuch)
{
	if ((howmuch>0)&&(page+cursor!=num-1)) {
		if ((cursor+howmuch)<rows) {
			if ((page+cursor+howmuch)<num) {
				removecursor();
				cursor+=howmuch;
				textattr(bcol|((taglist[page+cursor])?8:0));
				gotoxy(1,cursor+1+FROMUP);
				showrow(page+cursor);
			} else {
				cursor=num-1-page;
				show();
			}
		} else {
			if ((page+rows-1+howmuch)<num) {
				page+=howmuch;
				show();
			} else {
				if ((num-rows)>0) {
					page=num-rows;
					cursor=rows-1;
					show();
				} else {
					page=0;
					cursor=num-1;
					show();
				}
			}
		}
	}


	if ((howmuch<0)&&(page+cursor!=0)) {
		if ((cursor+howmuch)>=0) {
			removecursor();
			cursor+=howmuch;
			textattr(bcol|((taglist[page+cursor])?8:0));
			gotoxy(1,cursor+1+FROMUP);
			showrow(page+cursor);
		} else {
			if ((page+howmuch)>=0) {
				page+=howmuch;
				show();
			} else {
				page=0;
				cursor=0;
				show();
			}
		}
	}

}

// homes cursor.
void filelist::home()
{
	page=0;
	cursor=0;
}

// cursor to the end of the filelist.
void filelist::end()
{
	if ((num-rows)>0) {
		page=num-rows;
		cursor=rows-1;
	} else {
		page=0;
		cursor=num-1;
	}
}

// returns position of the curosr because is a private and should not be
// change by external funcs.
int filelist::returnpos()
{
	return(page+cursor);
}


// tags a line and moves to the next line.
void filelist::tag()
{
	if ((list[page+cursor].ff_attrib!=FA_DIREC)||(strcmp(list[page+cursor].ff_name,"..")!=0)) {
		if(taglist[page+cursor]) {
			taglist[page+cursor]=0;
			tagnum--;
			tagsize-=list[page+cursor].ff_fsize;
		} else {
			taglist[page+cursor]=1;
			tagnum++;
			tagsize+=list[page+cursor].ff_fsize;
		}
		gotoxy(1,cursor+1+FROMUP);
		if ((page+cursor)!=num-1) textattr( col|((taglist[page+cursor])?8:0));
		else			  textattr(bcol|((taglist[page+cursor])?8:0));
		showrow(page+cursor);
		if ((page+cursor)!=num-1)movepos(+1);
		updatebottom();
	}
}

// called to update the tag summery at the bottom. and the Loc/Far LED. :)
void filelist::updatebottom()
{
	int x=wherex(),y=wherey();
	textattr(0x1e);
	gotoxy(1,rows+FROMUP+1);
	if (tagnum) {
		cprintf("Tagged: %3d ",tagnum);
		if (cols>=3) cprintf(" Total size:%9ld ",tagsize);
	}
	else {
		cprintf("No tagged.  ");
		if (cols>=3) cprintf("                      ");
	}

	if (farcomp) {
		textattr(0x1c);
		cprintf("Far");
	} else {
		textattr(0x14);
		cprintf("Loc");
	}

	gotoxy(x,y);
}

// '*' function.
void filelist::logicalnottag()
{
	int i;
	for (i = 0 ; i < num ; i++)
		if ((list[i].ff_attrib!=FA_DIREC)||(strcmp(list[i].ff_name,"..")!=0)) {
			if (taglist[i]) {
				taglist[i]=0;
				tagsize-=list[i].ff_fsize;
				tagnum--;
			} else {
				taglist[i]=1;
				tagsize+=list[i].ff_fsize;
				tagnum++;

			}
		}

}

// '+' function.
void filelist::tagall()
{
	int i;
	long s=0;
	int n=0;
	for (i = 0 ; i < num ; i++)
		if ((list[i].ff_attrib!=FA_DIREC)||(strcmp(list[i].ff_name,"..")!=0)) {
			taglist[i]=1;
			n++;
			s+=list[i].ff_fsize;
		}
	tagnum=n;
	tagsize=s;
}

// uses just for the sort function.
void filelist::swaprecord(int s,int d)
{
	struct ffblk t;
	char t2;

	t = list[d];
	list[d] = list[s];
	list[s] = t;
/*	t2 = taglist[d];
	taglist[d] = taglist[s];
	taglist[s] = t2;*/
}


// bouble sorts the list by it's sortby var.
void filelist::sort()
{
	int wheredirec=0;
	int i,j;
	struct ffblk t;
	char t2;

	// first by directory.
	for (i = 0 ; i < num ; i ++) {
		if (list[i].ff_attrib==FA_DIREC) {
			swaprecord(i,wheredirec);
			wheredirec++;
		}
	}

	switch (sortby) {
		// by name.
		case 1 :
			for (i = 0 ; i < wheredirec-1 ; i++)
				for (j = wheredirec-2 ; j >=i ; j--)
					if (strcmp(list[j].ff_name,list[j+1].ff_name)>0)
						swaprecord(j,j+1);

			for (i = wheredirec ; i < num-1 ; i++)
				for (j = num-2 ; j >=i ; j--)
					if (strcmp(list[j].ff_name,list[j+1].ff_name)>0)
						swaprecord(j,j+1);
			break;
		// by size. directorys by name.
		case 2 :
			for (i = 0 ; i < wheredirec-1 ; i++)
				for (j = wheredirec-2 ; j >=i ; j--)
					if (strcmp(list[j].ff_name,list[j+1].ff_name)>0)
						swaprecord(j,j+1);
			for (i = wheredirec ; i < num-1 ; i++)
				for (j = num-2 ; j >=i ; j--)
					if (list[j].ff_fsize>list[j+1].ff_fsize)
						swaprecord(j,j+1);
			break;
		// by date.
		case 3 :
			for (i = 0 ; i < wheredirec-1 ; i++)
				for (j = wheredirec-2 ; j >=i ; j--)
					if (list[j].ff_fdate>list[j+1].ff_fdate)
						swaprecord(j,j+1);
			for (i = wheredirec ; i < num-1 ; i++)
				for (j = num-2 ; j >=i ; j--)
					if (list[j].ff_fdate>list[j+1].ff_fdate)
						swaprecord(j,j+1);
			break;
		// by time.
		case 4 :
			for (i = 0 ; i < wheredirec-1 ; i++)
				for (j = wheredirec-2 ; j >=i ; j--)
					if (list[j].ff_ftime>list[j+1].ff_ftime)
						swaprecord(j,j+1);
			for (i = wheredirec ; i < num-1 ; i++)
				for (j = num-2 ; j >=i ; j--)
					if (list[j].ff_ftime>list[j+1].ff_ftime)
						swaprecord(j,j+1);
			break;


		default : break;     // or just by directory attribute.
	}
}
#endif