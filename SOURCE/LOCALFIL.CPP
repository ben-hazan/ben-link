#ifndef __LOCALFIL_CPP
#define __LOCALFIL_CPP

#ifndef __STDIO_H
#include <stdio.h>
#endif

#ifndef __DIR_H
#include <dir.h>
#endif

#ifndef __IO_H
#include <io.h>
#endif

#ifndef __TEXTWIN_CPP
#include <textwin.cpp>
#endif

/* Scan [max] files to ffblk array [files] */
/* Returns number of files found */
int scanfiles(struct ffblk *files,unsigned int max,const char *wild,int attr)
{
int i,j;
struct ffblk blk;
	j = 0;
	i = findfirst(wild,&blk,attr);
	while ((!i)&&(j<max))
	{
		files[j]=blk;
		i = findnext(&blk);
		if (!((files[j].ff_name[0]=='.')&&(files[j].ff_name[1]==NULL)))j++;
	}
	return j;
}

// checks for file existance.
int fileexists(char *filename)
{
  return (access(filename, 0) == 0);
}

// returns available drives.
void returnlocaldrives(char *drives)
{
	int save,disk;

	save = getdisk();

	for (disk = 0; disk < 26 ;disk++) {
		setdisk(disk);
		if (disk == getdisk())drives[disk]=1;
		else	   	      drives[disk]=0;
	}
	drives[save]=2;
	setdisk(save);
}

// returns file size and return to it's beggining.
long returnfilesize(FILE *tofile)
{
	long l;
	fseek(tofile,0,SEEK_END);
	l = ftell(tofile);
	fseek(tofile,0,SEEK_SET);
	return l;
}

// opens copy window. local copys a file using full file names.
int localcopy(char *source, char *dest)
{
	float l;
	long sum=0;
	FILE *s,*d;
	char buf[512];
	int i,j;
	textwin *w;

	w = new textwin(25,15,30,3,0x3e,0x3f,"COPYING");
	if (w==NULL) return 1;
	w->show(1);

	cprintf("Starting to copy.");

	// open files.
	if ((s = fopen(source,"rb")) == NULL) {
		//msgbox("Error","Can't open source file.");
		delete w;
		return 2;
	}
	if ((d = fopen(dest,"wb")) == NULL) {
		//msgbox("Error","Can't create destination file.");
		delete w;
		fclose(s);
		return 3;
	}

	clrscr();

	if (strlen(source)>25)
		cprintf("...%s ==>\n\r",source+strlen(source)-22);
	else cprintf("%s ==>\n\r",source);
	if (strlen(dest)>25)
		cprintf("...%s þùú\n\r",dest+strlen(dest)-22);
	else cprintf("%s þùú\n\r",dest);

	cprintf("Precent done: Zero.");

	l = returnfilesize(s);
	if (l==0) {
		delete w;
		fclose(s);
		fclose(d);
		return 0;
	}

	do {
		// read from file.
		i = fread(buf,1,512,s);
		j = fwrite(buf,1,i,d);
		if (j!=i) {
			//msgbox("Error","Unknown error while copying.");
			fclose(s);
			fclose(d);
			delete w;
			return 4;
		} else {
			// print precents.
			sum+=j;
			gotoxy(1,wherey());
			printf("Precent done: ");
			printf("%.0f% ",(float (sum/l))*100);
			if ((sum/512/4+1)%4==0) printf("-");
			if ((sum/512/4+1)%4==1) printf("\\");
			if ((sum/512/4+1)%4==2) printf("|");
			if ((sum/512/4+1)%4==3) printf("/");
		}
	} while (i != 0);

	fclose(s);
	fclose(d);

	delete w;

	return 0;
}


/*-------------------------------- Higher level deleting ----------------*/
// avery simple two functions recursive operation.

// deletes a files + prompt. if it's a directory calls dirdel().
int kill(struct ffblk*,int *last);


// enters a directory, scan the files, on each file calls kill().
int dirdel(char *dir, int *last)
{
	struct ffblk dirblock;
	int done;

	static int dirlevel=0;

	// enters directory.
	if (dir[0]!='.') {
		if(chdir(dir)!=0)return 1;
		dirlevel++;
	} else return 1;

	// scan files.
	done = findfirst("*.*",&dirblock,FA_DIREC);
	while (!done) {
		// calls kill.
		kill(&dirblock,last);
		done = findnext(&dirblock);
	}
	// exits directory.
	if (dirlevel>0) {
		chdir("..");
		dirlevel--;
	}
	return 0;
}

int kill(struct ffblk *b,int *last)
{
	char buttons[4][BUTTONSIZE] = {
		"Delete","Skip","Cancel","All"};
	textwin *w;
	int i,t;

	if (*last==2) return 0;
	t = *last;
	if ((b->ff_attrib==FA_DIREC)&&(b->ff_name[0]!='.')) {
		dirdel(b->ff_name,last);
		rmdir(b->ff_name);
	}
	// propts and deletes.
	// 0 - delete (or overwrite in copy)
	// 1 - skip
	// 2 - cancel ( the whole operation.
	// 3 - all (delete all files).
	if (b->ff_attrib!=FA_DIREC) {
		if (*last == 3) {
			w = new textwin(25,7,36,2,0x3e,0x3f,"DELETING");
			if (w==NULL) return 1;
			w->show(1);
			cprintf("Deleting: %s\n\r",b->ff_name);
			if (unlink(b->ff_name)!=0) {
				msgbox("Disk error","Failed deleting file.");
				t=0;
			}
			delete w;
		}
		if ((*last == 0)||(*last == 1)) {
			w = new textwin(25,7,36,2,0x3e,0x3f,"DELETING");
			if (w==NULL) return 1;
			w->show(1);
			cprintf("Deleting: %s\n\r",b->ff_name);
			t = choosebutton(4,buttons,0x3f,0x4f);
			if ((t==0)||(t==3)) {
				if (unlink(b->ff_name)!=0) {
					msgbox("Disk error","Failed deleting file.");
					t=0;
				}
			}
			delete w;
		}
		*last=t;
                if (*last<0) *last=2;
	}
	return 0;
}

#endif
