#ifndef __FARFILES_CPP
#define __FARFILES_CPP

#ifndef __STDIO_H
#include <stdio.h>
#endif

#ifndef __STRING_H
#include <string.h>
#endif

#ifndef __IO_H
#include <io.h>
#endif

#ifndef __FILELIST_CPP
#include <filelist.cpp>
#endif

#ifndef __LOCALFIL_CPP
#include <localfil.cpp>
#endif

#ifndef __LPT_CPP
#include <lpt.cpp>
#endif

#ifndef __DEFINES_CPP
#include <defines.cpp>
#endif


// NOTE: u should never try to send one character over the port.
// LPT functions never initialize the port after such transfer.

/*------------------------- send a type over port ------------------------*/

// all send a type functions return 0 on process went fine.
// all send a type functions initialize port on error and return non 0.
// these functions use the maxwait definition. not needed in other funcs.

// send an int over port.
int sendint(int d)
{
	int i,c;
	// loop maxwait times try to send the int.
	for (i = 0 ; i < MAX_WAIT ; i++)
		if ((c = pp_send_block(2,(unsigned char*)&d))==OK)break;
	if (c != OK) {
		pp_init(LPTNUM);
		return 1;
	}
	return 0;
}

// reads an int from port.
int readint(int *d)
{
	int i,c,l;
	l = 2; // Integer limit.
	for (i = 0 ; i < MAX_WAIT ; i++)
		if ((c = pp_read_block(&l,(unsigned char*)d))==OK)break;
	if (c != OK) {
		pp_init(LPTNUM);
		return 1;
	}

	if (l!=2) {
		pp_init(LPTNUM);
		return 2;
	}

	return 0;
}

// reads a string from port.
// null included and add --> thats because --> look note above.
int readstr(char *str,int limit)
{
	int i,c,l;
	l=limit;
	for (i = 0 ; i < MAX_WAIT ; i++)
		if ((c = pp_read_block(&l,str))==OK) break;
	if (c!=OK) {
		pp_init(LPTNUM);
		return 1;
	}

	str[l-1]=NULL;

	return 0;
}

// sends a string on port (null terminated).
int sendstr(char *str)
{
	int i,c;

	for (i = 0 ; i < MAX_WAIT ; i++)
		if ((c = pp_send_block(strlen(str)+1,str))==OK) break;
	if (c!=OK) {
		pp_init(LPTNUM);
		return 1;
	}
	return 0;
}


// sends an ffblk on port.
int sendffblk(struct ffblk *blk)
{
	int i,c;
	for (i=0;  i<MAX_WAIT; i++)
		if ((c=pp_send_block(sizeof(struct ffblk),(unsigned char *)blk))==OK)break;

	if (c!=OK){
		pp_init(LPTNUM);
		return 1;
	}

	return 0;
}

// reads an ffblk from port
int readffblk(struct ffblk *blk)
{
	int i,c,s;
	s = sizeof(struct ffblk);
	for (i=0;  i<MAX_WAIT; i++)
		if ((c=pp_read_block(&s,(unsigned char *)blk))==OK)break;

	if (c!=OK) {
		pp_init(LPTNUM);
		return 1;

	}

	if (s!=sizeof(struct ffblk)) {
		pp_init(LPTNUM);
		return 2;

	}
	return 0;
}

/*- Hopa, here's a difference.-*/
/*- these function opens a files which is defined in an ffblk and send it
over port. that's it. -*/
int transmit_file(ffblk *file)
{
	int i,j;
	FILE *tofile;
	char buf[T_BLOCK_SIZE];  // trans buffer.
	char reply[MAXCOMMAND];  // reply buffer.
	long size;		 // size remained.
	int transsize;		 // size to transfer this loop.
	textwin *w;		 // window buffer.

	w = new textwin(25,15,30,3,0x3e,0x3f,"UPLOADING");
	if (w==NULL) return 1;
		// show a window.

	w->show(1);
	cprintf("Starting transmittion.");

	if (sendffblk(file)) {
		//msgbox("Error","Could not send initial data.");
		delete w;
		return 2;
	}
	// send definitions of the files.

	size = file->ff_fsize;
	if ((tofile = fopen(file->ff_name,"rb"))==NULL) {
		//msgbox("Error","Can not open file.");
		delete w;
		return 3;
	}
	// open file for reading only.

	clrscr();
	if (strlen(file->ff_name)>25)
		cprintf("...%s ==>\n\r",file->ff_name+strlen(file->ff_name)-22);
	else cprintf("%s ==>\n\r",file->ff_name);
	cprintf("To far computer.\n\rPrecent done: Zero.");

	while (size > 0) {

		transsize = (size > T_BLOCK_SIZE)? T_BLOCK_SIZE : size;
		if (fread(buf,1,transsize,tofile)!=transsize) {
			//msgbox("Error","Could not read from file.");
			fclose(tofile);
			delete w;
			return 4;
		}
		// read from file.
		do {
			if (pp_send_block(transsize, buf)) {
				//msgbox("Error","Could not transmit.");
				fclose(tofile);
				delete w;
				return 5;
			}

			if (readstr(reply,MAXCOMMAND)) {
				//msgbox("Error","Could not receive reply.");
				fclose(tofile);
				delete w;
				return 6;
			}
		} while (strcmp(reply,"Repeat.")==0);
		// send till u get OK reply.

		if (strcmp(reply,"OK.")!=0) {
			msgbox("Error","Error in reply.");
			fclose(tofile);
			delete w;
			return 7;
		}
		size -= transsize;
		gotoxy(1,wherey());
		printf("Precent done: ");
		printf("%.2f%    ",(float(float(file->ff_fsize-size))/file->ff_fsize)*100);
		if ((size/T_BLOCK_SIZE)%4==0) printf("-");
		if ((size/T_BLOCK_SIZE)%4==1) printf("\\");
		if ((size/T_BLOCK_SIZE)%4==2) printf("|");
		if ((size/T_BLOCK_SIZE)%4==3) printf("/");
		// print precents.
	}
	fclose(tofile);
	delete w;
	return 0;
}

/*- really similar to the previous functiond. just reads from port instead of
transmitting.-*/
int receive_file()
{
	FILE *tofile;
	char buf[T_BLOCK_SIZE];
	struct ffblk blk;
	long size;
	int i,j,n;
	textwin *w;

	w = new textwin(25,15,30,3,0x3e,0x3f,"DOWNLOADING");
	if (w==NULL) return 1;


	w->show(1);
	cprintf("Starting transmittion.");

	if (readffblk(&blk)) {
		//msgbox("Error","Could not receive initial data.");
		delete w;
		return 2;
	}
	size = blk.ff_fsize;
	if ((tofile = fopen(blk.ff_name,"wb"))==NULL) {
		//msgbox("Error","Can not open file.");
		delete w;
		return 3;
	}

	clrscr();
	cprintf("From far computer ==>\n\r");
	cprintf("%s þùú\n\r",blk.ff_name);
	cprintf("Precent done: Zero.");

	while (size > 0) {

		for (i=0 ; i<MAX_WAIT; i++) {
			n=T_BLOCK_SIZE;
			j = pp_read_block(&n, buf);
			if (j != CHAR1_TIMEOUT) break;
		}

		if (j == CSUM_FAIL || j == COUNT_FAIL) {
			if (sendstr("Repeat.")) {
				//msgbox("Error","Can not send command repeat.");
				fclose(tofile);
				delete w;
				return 4;
			}
			continue;
		} else if (j == OK) {
			if (fwrite(buf,1,n,tofile)!=n) {
				//msgbox("Error","Can not write to file.");
				fclose(tofile);
				delete w;
				return 5;
			}
			if (sendstr("OK.")) {
				//msgbox("Error","Can not send command ok.");
				fclose(tofile);
				delete w;
				return 6;
			}
			size -= n;
			gotoxy(1,wherey());
			printf("Precent done: ");
			printf("%.2f%    ",(float (float(blk.ff_fsize-size)/blk.ff_fsize))*100);
			if ((size/T_BLOCK_SIZE)%4==0) printf("-");
			if ((size/T_BLOCK_SIZE)%4==1) printf("\\");
			if ((size/T_BLOCK_SIZE)%4==2) printf("|");
			if ((size/T_BLOCK_SIZE)%4==3) printf("/");
		}
	}
	fclose(tofile);
	delete w;
	return 0;
}



/*---------------------------- far functions ----------------------------*/
/* which does not need a buffer */



/*------------- Standard C/C++ library functions. translated by me. -----*/

// all of the FARxxxxxxx functions are called from the clients computer
//     in order to make the xxxxxxxxx function on the far computer and
//     get the returned value from there.
// all of the REMOTExxxxxxxxx functions are called from the server function
//     and are called when an "xxxxx()" string is sent to them. they return.
//     the requested function return value to the port.
// these function also returns a non zero to the activator on error.

int farunlink(char *filename,int *ret)
{
	if (sendstr("Unlink()")) return 1;
	if (sendstr(filename)) return 2;
	if (readint(ret)) return 3;
	return 0;
}

int remoteunlink()
{
	char filename[13];
	int ret;
	if (readstr(filename,13)) return 1;
	ret = unlink(filename);
	if (sendint(ret)) return 2;
	return 0;
}

int farchdir(char *path,int *ret)
{
	if (sendstr("Chdir()")) return 1;
	if (sendstr(path)) return 2;
	if (readint(ret)) return 3;
	return 0;
}

int remotechdir()
{
	char path[MAXPATH];
	int ret;
	if (readstr(path,MAXPATH)) return 1;
	ret = chdir(path);
	if (sendint(ret)) return 2;
	return 0;
}


int fargetcwd(char *buf, int buflen)
{
	if (sendstr("Getcwd()")) return 1;
	if (readstr(buf,buflen)) return 2;
	return 0;
}

int remotegetcwd()
{
	char path[MAXPATH];
	getcwd(path,MAXPATH);
	if (sendstr(path))return 1;
	return 0;
}


int farmkdir(char *path,int *ret)
{
	if (sendstr("Mkdir()")) return 1;
	if (sendstr(path)) return 2;
	if (readint(ret)) return 3;
	return 0;
}

int remotemkdir()
{
	char path[MAXPATH];
	int ret;
	if (readstr(path,MAXPATH)) return 1;
	ret = mkdir(path);
	if (sendint(ret)) return 2;
	return 0;
}

int farrmdir(char *path,int *ret)
{
	if (sendstr("Rmdir()")) return 1;
	if (sendstr(path)) return 2;
	if (readint(ret)) return 3;
	return 0;
}

int remotermdir()
{
	char path[MAXPATH];
	int ret;
	if (readstr(path,MAXPATH)) return 1;
	ret = rmdir(path);
	if (sendint(ret)) return 2;
	return 0;
}

int farfindfirst(char *pathname, struct ffblk *blk, int attrib,int *ret)
{
	if (sendstr("Findfirst()")) return 1;
	if (sendstr(pathname)) return 2;
	if (sendint(attrib)) return 3;
	if (readffblk(blk)) return 4;
	if (readint(ret)) return 5;
	return 0;
}

int remotefindfirst()
{
	char path[MAXPATH];
	int attrib;
	struct ffblk blk;
	int ret;
	if (readstr(path,MAXPATH)) return 1;
	if (readint(&attrib)) return 2;
	ret = findfirst(path,&blk,attrib);
	if (sendffblk(&blk)) return 3;
	if (sendint(ret)) return 4;
	return 0;
}

int farfindnext(struct ffblk *blk,int *ret)
{
	if (sendstr("Findnext()")) return 1;
	if (sendffblk(blk)) return 2;
	if (readffblk(blk)) return 3;
	if (readint(ret)) return 4;
	return 0;
}

int remotefindnext()
{
	struct ffblk blk;
	int ret;
	if (readffblk(&blk)) return 1;
	ret = findnext(&blk);
	if (sendffblk(&blk)) return 2;
	if (sendint(ret)) return 3;
	return 0;
}


/*- uses for userchangedisk function:
	send command to far computer to change disk -*/
int farsetdisk(int disk, int *ret)
{
	if (sendstr("Setdisk()")) return 1;
	if(sendint(disk)) return 2;
	if(readint(ret)) return 3;
	return 0;
}

int remotesetdisk()
{
	int disk;
	int ret;
	if (readint(&disk)) return 1;
	ret = setdisk(disk);
	if (sendint(ret)) return 2;
	return 0;
}

int fargetdisk(int *ret)
{
	if (sendstr("Getdisk()")) return 1;
	if (readint(ret)) return 2;
	return 0;
}

int remotegetdisk()
{
	int ret;
	ret = getdisk();
	if (sendint(ret)) return 1;
	return 0;
}

/*----------------------------- My functions. ----------------------------*/

// gets available drives on far computer: 26 chars.
// similar to getlocaldrives :)__| dugh  (in localfil.cpp)
int fargetdrives(char *drives)
{
	int i,c,l;
	l = 26; // number of letters
	if (sendstr("Getdrives()")) return 1;
	for (i = 0 ; i < MAX_WAIT ; i++)
		if ((c = pp_read_block(&l,drives))==OK) break;
	if ((c!=OK)||(l!=26)) {
		pp_init(LPTNUM);
		return 2;
	}
	return 0;
}

int remotegetdrives()
{
	int i,c;

	char drives[26];
	returnlocaldrives(drives);
	for (i = 0 ; i < MAX_WAIT ; i++)
		if ((c = pp_send_block(26,drives))==OK) break;
	if (c!=OK) {
		pp_init(LPTNUM);
		return 1;
	}
	return 0;
}

// similar to scanfiles in localfil.cpp just does it on the far computer.
int farscanfiles(struct ffblk *files,unsigned int max,char *wild,int attr,int *ret)
{
int i,j;
struct ffblk ffblk;
	j = 0;
	if(farfindfirst(wild,&ffblk,attr,&i))return 1;
	while ((!i)&&(j<max))
	{
		files[j]=ffblk;
		if (farfindnext(&ffblk,&i))return 2;
		if (!((files[j].ff_name[0]=='.')&&(files[j].ff_name[1]==NULL)))j++;
	}
	*ret = j;
	return 0;
}

// similar to fileexists on localfil.cpp
int farfileexists(char *filename,int *ret)
{
	if (sendstr("Fileexists()")) return 1;
	if (sendstr(filename)) return 2;
	if (readint(ret)) return 3;
	return 0;
}

int remotefileexists()
{
	char filename[MAXPATH+13];
	int ret;
	if (readstr(filename,MAXPATH+13)) return 1;
	ret = fileexists(filename);
	if (sendint(ret)) return 2;
	return 0;
}

// initializes transmition in both computers. this one downloads a file.
int fartransmit_file(struct ffblk *file,int *ret)
{
	if (sendstr("Transmit_file()")) return 1;
	if (sendffblk(file)) return 2;
	if (receive_file()) return 3;
	if (readint(ret)) return 4;
	return 0;
}

int remotetransmit_file()
{
	int ret;
	struct ffblk blk;
	if (readffblk(&blk))return 1;
	ret = transmit_file(&blk);
	if (sendint(ret)) return 2;
	return 0;
}

// initializes transmition in both computers. this one uploads a file.
int farreceive_file(struct ffblk *file,int *ret)
{
	if (sendstr("Receive_file()")) return 1;
	if (transmit_file(file)) return 2;
	if (readint(ret)) return 3;
	return 0;
}

int remotereceive_file()
{
	int ret;
	ret = receive_file();
	if (sendint(ret)) return 1;
	return 0;
}

// runs local copy on a far computer :^.
int farlocalcopy(char *source, char *dest)
{
	textwin *w;
	char str[3];
	int i;

	w = new textwin (17,5,47,1,0x3e,0x3f,"Waiting");

	if (sendstr("Localcopy()")) return 1;
	if (sendstr(source)) return 2;
	if (sendstr(dest)) return 3;

	w->show(1);
	cprintf("Waiting for other computer to finish his job.");

	do {
		i = readstr(str,3);
	} while (i!=0);

	delete w;

	if (strcmp("OK",str))
		msgbox("Transmit error","Error while waiting for far computer.");

	return 0;
}

int remotelocalcopy()
{
	char source[MAXPATH+13];
	char dest[MAXPATH+13];
	char filename[13];

	if (readstr(source,MAXPATH)) return 1;
	if (readstr(dest,MAXPATH)) return 2;
	localcopy(source,dest);
	sendstr("OK");
	return 0;
}

#endif


/*--------------------------- higher level deleting ----------------------*/
// just like kill and dirdel these are farkill 7 fardirdel.
// they use FARxxxx functions instead of xxxx functions.
int farkill(struct ffblk*,int *last);


int fardirdel(char *dir, int *last)
{
	int i;
	struct ffblk dirblock;
	int done;

	static int dirlevel=0;

	if (dir[0]!='.') {
		if (farchdir(dir,&i)) return 1;
		if(i!=0)return 1;
		dirlevel++;
	} else return 1;

	if (farfindfirst("*.*",&dirblock,FA_DIREC,&done)) {
		msgbox("Transmit error","Failed receiving information.");
		return 2;
	}
	while (!done) {
		if (farkill(&dirblock,last)!=0)return 2;
		if (farfindnext(&dirblock,&done)) {
			msgbox("Transmit error","Failed receiving information.");
			return 2;
		}
	}
	if (dirlevel>0) {
		if (farchdir("..",&i)) {
			msgbox("Transmit error","Failed changing directory.");
			return 2;
		}
		dirlevel--;
	}
	return 0;
}



int farkill(struct ffblk *b,int *last)
{
	char buttons[4][BUTTONSIZE] = {
		"Delete","Skip","Cancel","All"};
	textwin *w;
	int i,t;

	if (*last==2) return 0;
	t=*last;
	if ((b->ff_attrib==FA_DIREC)&&(b->ff_name[0]!='.')) {
		if (fardirdel(b->ff_name,last)==2) return 2;
		if (farrmdir(b->ff_name,&i)) {
			msgbox("Transmit error","Failed removing directory.");
			return 2;
		}
	}
	if (b->ff_attrib!=FA_DIREC) {
		if (*last == 3) {
			w = new textwin(25,7,36,2,0x3e,0x3f,"DELETING");
			if (w==NULL) return 1;
			w->show(1);
			cprintf("Deleting: %s",b->ff_name);
			if (farunlink(b->ff_name,&i)) {
				msgbox("Transmit error","Failed deleting file.");
				return 2;
			}
			if (i!=0) {
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
				if (farunlink(b->ff_name,&i)) {
					msgbox("Transmit error","Failed deleting file.");
					return 2;
				}
				if (i!=0) {
					msgbox("Disk error","Failed deleting file.");
					t=0;
				}
			}
			delete w;
		}
		*last=t;
                if (*last<0)*last=2;
	}
	return 0;
}
