/*- functionality -*/
#ifndef __FUNCTION_CPP
#define __FUNCTION_CPP


#ifndef __PROCESS_H
#include <process.h>
#endif
#ifndef __INIT_CPP
#include <init.cpp>
#endif
#ifndef __TEXTWIN_CPP
#include <textwin.cpp>
#endif
#ifndef __FILELIST_CPP
#include <filelist.cpp>
#endif
#ifndef __FARFILES_CPP
#include <farfiles.cpp>
#endif

// called when users changes the directory (presses ENTER).
int userchangedirectory(filelist *lisp)
{
	int ret;
	if (lisp->list[lisp->returnpos()].ff_attrib==FA_DIREC) {
	// is it a directory?
		if (lisp->farcomp) {
		// is this list logged to far computer?
		// yes: Send command to change directory.
			if (farchdir(lisp->list[lisp->returnpos()].ff_name,&ret)) {
			// an error accured sending directory's information.
				msgbox("Transmit error","Failed changing directory. Reading local directory.");
				resetlist(lisp);
			}
		} else { // On local computer:
			chdir(lisp->list[lisp->returnpos()].ff_name); // change dir.
			getcwd(lisp->path,MAXPATH);   // get path.
		}
		if (updatelist(lisp)) msgbox("Transmit error","Failed receiving directory data. Reading local directory.");
	}
	return 0;
}


/*- opens a windows, lets the user choose drive -*/
int userchangedrive(filelist *l)
{
	char buttons[26][BUTTONSIZE];

	int i,j;
	char drives[26];
	unsigned char k;
	textwin *d;

	// Scan for available drives on the current machine.
	if (lisp->farcomp) {
		if (fargetdrives(drives)) {
			msgbox("Transmit error","Failed receiving availabe drives. Reading local directory.");
			resetlist(l);
			updatelist(l);
			delete d;
			return 2;
		}
	} else 	returnlocaldrives(drives);

	j= 0;
	for (i = 0 ; i < 26 ; i++) {
		if (drives[i]) {
			buttons[j][0]=i+'A';
			buttons[j][1]=':';
			buttons[j][2]=NULL;
			j++;
		}
	}

	d = new textwin(31,5,18,j/3+1,0x5d,0x5e,"Choose drive:");
	if (d==NULL) return 1;

	// Shows a window with the drives.
	d->show(1);
	textattr(0x5d);

	i = choosebutton(j,buttons,0x5e,0x4f);

	// Let you choose.

	if (i<0){
		delete d;
		return 3;
	}
	// changes drive number in the file list class.
	lisp->drive=buttons[i][0]-'A';

	// is drive on far computer?
	if (lisp->farcomp) {
		//yes : command to change drive.
		if (farsetdisk(lisp->drive,&i)) {
			msgbox("Transmit error","Setting drive failed. Reading local drive.");
			resetlist(l);
			updatelist(l);
			delete d;
			return 4;
		}
	} else {
		// on local machine:
		setdisk(lisp->drive); // just change drive.
		resetlist(l);
	}
	if(updatelist(l)) msgbox("Transmit error","Could not read far drive. Reading local directory.");

	delete d;
	return 0; // return OK.
}

// opens server window. enters server mode.
int server()
{
	textwin *server;
	server = new textwin(22,6,35,1,0x3f,0x3f,"Server mode, Press F10 to quit.");
	if (server==NULL) return 1;

/*--------------------------- Initializing server ----------------------*/

	int i,c;
	char command[MAXCOMMAND];
	unsigned char k;

	server->show(1);

	cprintf("\n\rPress F10 to quit.");
	if (pp_init(LPTNUM)!=OK){
		msgbox("Error","Could not initialize port. Check configuration.");
		_setcursortype(_NOCURSOR);
		delete server;
		return 1;
	}

/*-------------------------- Main server loop -----------------------*/

	while(1) {
		if (readstr(command,MAXCOMMAND)) {
			if ((k = getkey())!=0) {
				if (k == F0) {
					pp_init(LPTNUM);
					break;
				}
			}
			cprintf("\n\rIdle.");
		} else {
			// These elseif structure is like switch :) but better.
			// these list of functions has a remote functions activated for every farxxxx string sent.
			if (strcmp(command,"Unlink()")==0) {
				cprintf("\n\rDeleting.");
				if (remoteunlink()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Chdir()" )==0) {
				cprintf("\n\rChanging directory.");
				if (remotechdir()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Getcwd()")==0) {
				cprintf("\n\rTransmitting work directory.");
				if (remotegetcwd()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Mkdir()" )==0) {
				cprintf("\n\rMaking a new directory.");
				if (remotemkdir()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Rmdir()" )==0) {
				cprintf("\n\rRemoving directory.");
				if (remotermdir()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Findfirst()")==0) {
				cprintf("\n\rScaning directory.");
				if (remotefindfirst()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Findnext()")==0) {
				cprintf("\n\rScaning directory.");
				if (remotefindnext()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Setdisk()")==0) {
				cprintf("\n\rChanging drive.");
				if (remotesetdisk()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Getdisk()")==0) {
				cprintf("\n\rTransmitting drive.");
				if (remotegetdisk()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Getdrives()")==0) {
				cprintf("\n\rTransmitting available drives.");
				if (remotegetdrives()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Transmit_file()")==0) {
				cprintf("\n\rSending file.");
				if (remotetransmit_file()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Receive_file()")==0) {
				cprintf("\n\rReceiving file.");
				if (remotereceive_file()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Localcopy()")==0) {
				cprintf("\n\rCopying.");
				if (remotelocalcopy()) {
					delete server;
					return 2;
				}
			} else if (strcmp(command,"Fileexists()")==0) {
				cprintf("\n\rCheking file access.");
				if (remotefileexists()) {
					delete server;
					return 2;
				}
			} else cprintf("\n\rGarbage. Ignoring.");
		}
	}

/*------------------- Shutting server down ---------------------*/

	delete server;
	return 0;
}

/*- initialize compunications between far computer's server function to this
one's main.
-*/
int log(filelist *lisp)
{
	if (lisp->farcomp) {
		// logout.
		resetlist(lisp);
		updatelist(lisp);
	} else {
	  // Initialize port.
		if (pp_init(LPTNUM)) {
		// if error print error and don't switch mode.
			msgbox("Port error","Can't initialize port. Check configuration.");
			return 1;
		}
		// If port ok: command for file list.
		lisp->farcomp=1;
		if (updatelist(lisp)) {
			msgbox("Login error.","Could not login. Reading local directory.");
			return 2;
		}
	}
	return 0;
}

// spawns edit on users F4.
int useredit(filelist *l)
{
	char loaddir[MAXPATH+13];
	char buttons[2][BUTTONSIZE] = {
		"OK","Cancel"};
	int i;
	textwin *w;

	w = new textwin(12,5,55,2,0x4c,0x4e,"Edit");
	if (w == NULL) return 1;
	w->show(1);
	// open the prompting window

	strcpy(loaddir,DOSPATH);
	strcat(loaddir,"EDIT.COM");    // creat the full file name to run.

	// check where is the file.
	if (l->farcomp) {
		cprintf("Can not edit a file on far computer.\n\r			");
		choosebutton(1,buttons,0x4e,0x5f);
		delete w;
		return 0;
	}

	// check if it's a directory.
	if (l->list[l->returnpos()].ff_attrib==FA_DIREC) {
		cprintf("Can not edit directory.\n\r                        ");
		choosebutton(1,buttons,0x4e,0x5f);
		delete w;
		return 0;
	}

	// prompt.
	cprintf("Are you sure you want to edit: %s?\n\r                    ",l->list[l->returnpos()].ff_name);
	i = choosebutton(2,buttons,0x4e,0x5f);
	if (i==0) {
		if (spawnl(P_WAIT,loaddir,loaddir,l->list[l->returnpos()].ff_name,NULL)==-1) {
			msgbox("Spawning error","Could not run 'edit.com'. Check configuration.");
			delete w;
			return 1;
		}
	}
	delete w;
	_setcursortype(_NOCURSOR);
	resetlist(l);
	updatelist(l);
	return 0;
}

// reads a name, creats a new directory (user F7).
int usermkdir(filelist *l)
{
	textwin *w;
	char name[14];
	int i;

	// Prompt.
	w = new textwin(25,5,31,1,0x3c,0x3f,"Enter New directory's name:");
	if (w == NULL) return 1;
	w->show(1);
	cprintf("         ");
	name[0]=NULL;
	// get name.
	i = getstring(name,13);
	if (i<0) {
		delete w;
		return 0;
	}

	if (l->farcomp) {
		// on a far computer activate farxxxx functions.
		if (farmkdir(name,&i)) {
			msgbox("Transmit error","Failed sending command. Reading local directory.");
			resetlist(l);
			updatelist(l);
			delete w;
			return 2;
		}
		if (i!=0) {
			msgbox("Disk error","Failed creating directory.");
			updatelist(l);
			delete w;
			return 3;
		}
	} else {
		// call mkdir.
		if(mkdir(name)!=0) {
			msgbox("Disk error","Failed creating directory.");
			updatelist(l);
			delete w;
			return 4;
		}
	}

	// update filelist yaanu rescan files.
	if (updatelist(l)){
		msgbox("Transmit error","Failed reading far directory. Reading local directory.");
		delete w;
		return 5;
	}
	delete w;

	return 0;
}

// just prompt for quitting.
int userquit(int *e)
{
	textwin *w;
	char buttons[][BUTTONSIZE] = {
		"No","Yes"  };

	w = new textwin(23,10,35,2,0x3e,0x3f,"Quit");
	if (w == NULL) return 1;

	w->show(1);

	cprintf(" Are you sure you want to quit?\n\r            ");
	if (choosebutton(2,buttons,0x3f,0x4f)==1) *e=1;
	return 0;
}


// is called after copy, delete, move. because these functions may change
// both file list. (are called in main but not in function!!)
void updateboth(filelist *s, filelist *d)
{
	int i;
	if (d->farcomp) {
		if (farsetdisk(d->drive,&i)) {
			msgbox("Transmit error","Failed reading far directory. Reading loacl directory.");
			resetlist(d);
		} else {
			if (farchdir(d->path,&i)) {
				msgbox("Transmit error","Failed reading far directory. Reading loacl directory.");
				resetlist(d);
			} else {
				if (i!=0) {
					msgbox("Disk error","Failed reading far directory. Reading loacl directory.");
					resetlist(d);
				}
			}
		}
	} else {
		setdisk(d->drive);
		if (chdir(d->path)!=0) {
			msgbox("Disk error","Failed reading directory. Reading working directory.");
			resetlist(d);
		}
	}

	if (updatelist(d))
		msgbox("Transmit error","Failed reading far directory. Reading local directory.");

	if (s->farcomp) {
		if (farsetdisk(s->drive,&i)) {
			msgbox("Transmit error","Failed reading far directory. Reading loacl directory.");
			resetlist(s);
		} else {
			if (farchdir(s->path,&i)) {
				msgbox("Transmit error","Failed reading far directory. Reading loacl directory.");
				resetlist(s);
			} else {
				if (i!=0) {
					msgbox("Disk error","Failed reading far directory. Reading loacl directory.");
					resetlist(s);
				}
			}
		}
	} else {
		setdisk(s->drive);
		if (chdir(s->path)!=0) {
			msgbox("Disk error","Failed reading directory. Reading working directory.");
			resetlist(s);
		}
	}

	if (updatelist(s))
		msgbox("Transmit error","Failed reading far directory. Reading local directory.");
}



/*------------------------------ COPY function -------------------------------*/

// prompts for over writing.
int overwritebox(char *name,int *last)
{
	unsigned char k;
	textwin *w;
	char buttons[][BUTTONSIZE] = {
		"Overwrite","Skip","Cancel","All"};
	if (*last == 2) return 0;
	if (*last == 3) return 1;
	w = new textwin(15,5,45,2,0x3e,0x3f,"OVERWRITE");
	if (w == NULL) return -1;
	w->show(1);
	if (strlen(name)>20)
		cprintf("File ...%s already exists.\n\r",name+strlen(name)-20);
	else cprintf("File %s already exists.\n\r",name);
	*last = choosebutton(4,buttons,0x3f,0x4f);
	delete w;
	if (*last == 0) return 1;
	if (*last == 1) return 0;
	if (*last == 2) return 0;
	if (*last == 3) return 1;
        *last=2;
	return 0;
}


// copys uno filo.
// calls: localcopy/farlocalcopy/fartransmit_file/farreceive_file according the vars.
int copyfile(char *spath, char *sname, long sfsize, int scomp,char *dpath, int dcomp, int *last)
{
	char k;
	int i;
	char source[MAXPATH+13];
	char dest[MAXPATH+13];
	struct ffblk blk;

	// creats full names and the ffblk buffer.
	strcpy(source,spath);
	if (source[strlen(source)-1]!='\\') strcat(source,"\\");
	strcat(source,sname);
	strcpy(dest,dpath);
	if (dest[strlen(dest)-1]!='\\') strcat(dest,"\\");
	strcat(dest,sname);
	strcpy(blk.ff_name,sname);
	blk.ff_fsize=sfsize;

	// get full names.
	if (dcomp) {
	// destination is far (upload/far copy).

		//is the file already exist?
		// i means it does, i==0 it doesnt.
		if (farfileexists(dest,&i)) {
			msgbox("Transmit error","Failed accessing far computer.");
			return 1;
		}
		if (scomp) {
			// is the source on a far computer?
			if (i) {          // local copy on far computer.
				if (overwritebox(dest,last)){
					// prompt before copying.
					if (farlocalcopy(source,dest)) {
						msgbox("Transmit error","Failed sending command.");
						return 2;
					}
				}
			} else {
				if (farlocalcopy(source,dest)) {
					msgbox("Transmit error","Failed sending command.");
					return 3;
				}
			}
		} else {
			// source on the local computer.
			if (i) {
				if (overwritebox(blk.ff_name,last)) {
					// prompt before copying.
					if (farreceive_file(&blk,&i)) {
						msgbox("Transmit error","Failed transmiting file.");
						return 4;
					}
				}
			} else {
				if (farreceive_file(&blk,&i)) {
					msgbox("Transmit error","Failed transmiting file.");
					return 5;
				}
			}
		}
	} else {
	// destinaion is local(download/local copy).
		i = fileexists(dest);
		// does destination file already exist?

		if (scomp) {
			// is the source on a fer computer?
			if (i) {
				if (overwritebox(blk.ff_name,last)) {
					// prompt.
					if (fartransmit_file(&blk,&i)) {
						msgbox("Transmit error","Failed receiving file.");
						return 6;
					}
				}
			} else {
				if (fartransmit_file(&blk,&i)) {
						msgbox("Transmit error","Failed receiving file.");
						return 6;
				}
			}
		} else {
			// source on local computer.
			if (i) {
				if (overwritebox(dest,last)){
					// prompt.
					if(localcopy(source,dest)) {
						msgbox("Disk error","Failed copying.");
						return 3;
					}
				}
			} else {
				if(localcopy(source,dest)) {
					msgbox("Disk error","Failed copying.");
					return 3;
				}
			}
		}
	}
	return 0;
}

// copys the entire directory. this is like the dirdel algorithem so look there
// for more detailed AND SIMPLE information.

// enters dest directory. check existance of the dir and mk it and ch to it.
// then it scans the source directory and calls copyfile for each file.
int copydir(char *spath, char *dir, int scomp,char *dpath, int dcomp, int *last)
{
	int i;
	struct ffblk dirblock;
	int done;
	int ret;
	static int dirlevel=0;


	if (scomp) {
		if (dir[0]!='.') {
			if (farchdir(dir,&i)) return 1;
			if(i!=0)return 1;
			dirlevel++;
		} else return 1;
		if (fargetcwd(spath,MAXPATH)) {
			msgbox("Transmit error","Failed receiving information.");
			return 2;
		}
		if (dcomp) {
			if (farsetdisk(dpath[0]-'A',&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (farchdir(dpath,&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (farfileexists(dir,&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (ret==0) {
				if (farmkdir(dir,&ret)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
			}
			if (farchdir(dir,&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (fargetcwd(dpath,MAXPATH)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (farsetdisk(spath[0]-'A',&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (farchdir(spath,&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
		} else {
			if (!fileexists(dir)) {
				mkdir(dir);
			}
			chdir(dir);
			getcwd(dpath,MAXPATH);
		}

		if (farfindfirst("*.*",&dirblock,FA_DIREC,&done)) {
			msgbox("Transmit error","Failed receiving information.");
			return 2;
		}
		while (!done) {
			if (dirblock.ff_attrib==FA_DIREC) {
				if (dirblock.ff_name[0]!='.') {
					copydir(spath,dirblock.ff_name,scomp,dpath,dcomp,last);
				}
			} else {
				copyfile(spath,dirblock.ff_name, dirblock.ff_fsize, scomp, dpath, dcomp, last);
			}
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
			if (fargetcwd(spath,MAXPATH)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}

			if (dcomp) {
				if (farsetdisk(dpath[0]-'A',&ret)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
				if (farchdir(dpath,&ret)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
				if (farchdir("..",&i)) {
					msgbox("Transmit error","Failed changing directory.");
					return 2;
				}
				if (fargetcwd(dpath,MAXPATH)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
				if (farsetdisk(spath[0]-'A',&ret)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
				if (farchdir(spath,&ret)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
			} else {
				chdir("..");
				getcwd(dpath,MAXPATH);
			}
			dirlevel--;
		}

	} else {

		if (dir[0]!='.') {
			if(chdir(dir)!=0)return 1;
			dirlevel++;
		} else return 1;
		getcwd(spath,MAXPATH);

		if (dcomp) {
			if (farfileexists(dir,&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (ret==0) {
				if (farmkdir(dir,&ret)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
			}
			if (farchdir(dir,&ret)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
			if (fargetcwd(dpath,MAXPATH)) {
				msgbox("Transmit error","Failed receiving information.");
				return 2;
			}
		} else {
			setdisk(dpath[0]-'A');
			chdir(dpath);
			if (!fileexists(dir)) {
				mkdir(dir);
			}
			chdir(dir);
			getcwd(dpath,MAXPATH);
			setdisk(spath[0]-'A');
			chdir(spath);
		}

		done = findfirst("*.*",&dirblock,FA_DIREC);
		while (!done) {
			if (dirblock.ff_attrib==FA_DIREC) {
				if (dirblock.ff_name[0]!='.') {
					copydir(spath,dirblock.ff_name,scomp,dpath,dcomp,last);
				}
			} else {
				copyfile(spath,dirblock.ff_name, dirblock.ff_fsize, scomp, dpath, dcomp, last);
			}
			done = findnext(&dirblock);
		}
		if (dirlevel>0) {
			chdir("..");
			getcwd(spath,MAXPATH);
			if (dcomp) {
				if (farchdir("..",&i)) {
					msgbox("Transmit error","Failed changing directory.");
					return 2;
				}
				if (fargetcwd(dpath,MAXPATH)) {
					msgbox("Transmit error","Failed receiving information.");
					return 2;
				}
			} else {
				setdisk(dpath[0]-'A');
				chdir(dpath);
				chdir("..");
				getcwd(dpath,MAXPATH);
				setdisk(spath[0]-'A');
				chdir(spath);
			}
			dirlevel--;
		}
	}
	return 0;
}

// called on user press F5.
// copys cursored file or tagged files.
int usercopy(filelist *s,filelist *d)
{
	int all = 0;
	int i=0;
	if (s->tagnum) {
		while (all!=2) {  // while not cancel in delete.
			while ((i < s->num)&&(s->taglist[i]==0))i++;
			if (i >= s->num) break;
			if (s->list[i].ff_attrib==FA_DIREC) {
				copydir(s->path,s->list[i].ff_name,s->farcomp,d->path,d->farcomp,&all);
			} else {
				copyfile(s->path,s->list[i].ff_name,s->list[i].ff_fsize,s->farcomp,d->path,d->farcomp,&all);
			}
			i++;
		}
	} else {
		if (s->list[s->returnpos()].ff_attrib==FA_DIREC) {
			copydir(s->path,s->list[s->returnpos()].ff_name,s->farcomp,d->path,d->farcomp,&all);
		} else {
			copyfile(s->path,s->list[s->returnpos()].ff_name,s->list[s->returnpos()].ff_fsize,s->farcomp,d->path,d->farcomp,&all);
		}
	}

	return 0;
}




/*----------------------------- Delete function ---------------------------*/
// called on F8. uses kill/farkill & dirdel/fardirdel.

int userdelete(filelist *l)
{
	int all = 0;
	int i=0;
	if (l->farcomp) {
		if (l->tagnum) {
			while (all!=2) {  // while not cancel in delete.
				while ((i < l->num)&&(l->taglist[i]==0))i++;
				if (i >= l->num) break;
				farkill(&(l->list[i]),&all);
				i++;
			}
		} else {
			farkill(&(l->list[l->returnpos()]),&all);
		}
	} else {
		if (l->tagnum) {
			while (all!=2) {  // while not cancel in delete.
				while ((i < l->num)&&(l->taglist[i]==0))i++;
				if (i >= l->num) break;
				kill(&(l->list[i]),&all);
				i++;
			}
		} else {
			kill(&(l->list[l->returnpos()]),&all);
		}
	}

	return 0;
}


/*------------------------------ Move function --------------------------*/

// just calls copy and delete. not very complicated.
int usermove(filelist *s,filelist *d)
{
	if (usercopy(s,d)) {
		msgbox("Copy error","Will not delete files.");
		return 1;
	}

	if (userdelete(s)) {
		msgbox("Delete error","Not all files has been removed.");
		return 2;
	}
	return 0;
}


/*----------------------------- Functions without a hotkey ---------------*/

// a prompted chdir and rescan of files.
// called from menu.
int userpath(filelist *l)
{
	textwin *w;
	char path[MAXPATH];
	int i;

	w = new textwin(3,7,73,1,0x3c,0x3f,"Enter path:");
	if (w == NULL) return 1;
	w->show(1);
	path[0]=NULL;
	if (getstring(path,72)<0) {
		delete w;
		return 0;
	}
	if (l->farcomp) {
		if (farchdir(path,&i)) {
			msgbox("Transmit error","Failed transmiting path. Reading local directory.");
			resetlist(l);
			updatelist(l);
			return 2;
		}

	} else {
		if (chdir(path)!=0) {
			msgbox("Go to path error","Path not found.");
			resetlist(l);
			updatelist(l);
			return 3;
		}
		resetlist(l);
	}

	if (updatelist(l)) {
		msgbox("Transmit error","Failed reading far directory. Reading loacl directory.");
	}
	return 0;
}

// dos shells. means spawns command.com
int userdosshell()
{
	char loaddir[MAXPATH+13];
	char buttons[2][BUTTONSIZE] = {
		"OK","Cancel"};
	int i;
	textwin *w;

	w = new textwin(12,5,55,2,0x4c,0x4e,"Shelling to DOS");
	if (w == NULL) return 1;
	w->show(1);

	strcpy(loaddir,DOSPATH);
	strcat(loaddir,"COMMAND.COM");

	cprintf("Are you sure you want to shell to DOS?\n\r                    ");
	i = choosebutton(2,buttons,0x4e,0x5f);
	if (i==0) {
		window(1,1,80,25);
		textattr(0x07);
		clrscr();
		_setcursortype(_NORMALCURSOR);
		if (spawnl(P_WAIT,loaddir,loaddir,NULL)==-1) {
			msgbox("Spawning error","Failed loading 'Command.com'. Check configuration.");
			delete w;
			_setcursortype(_NOCURSOR);
			return 1;
		}
		_setcursortype(_NOCURSOR);
		print_hot_keys(0x3e,0x35); // Prints hotkeys line.
	}
	delete w;
	return 0;
}


// opens a help window.
int userhelpkeys()
{
	char button[][BUTTONSIZE] = {
		"OK"};
	textwin *w;

	w = new textwin(5,3,70,20,0x4e,0x4f,"Keys");
	if (w==NULL) return 1;

	w->show(1);

	cprintf("ENTER - Enter directory - Enters the pointer by the cursor directory.\n\r");
	cprintf(" TAB - Change list - Jumps from one file list to the other.\n\r\n\r");

	cprintf("SPACE - Tag - Tags a file.\n\r");
	cprintf(" + - Tag all - Tags all the files in the list.\n\r");
	cprintf(" - - Untag - Removes tags from all the files list.\n\r");
	cprintf(" * - Switch tag - Tags what's not tagged, Untags what's tagged.\n\r\n\r");

	cprintf("F1 - Change drive - Lets you choose the current window's drive.\n\r");
	cprintf("F2 - Server mode  - Enters server mode and waits for commands from \n\r                    far computer.\n\r");
	cprintf("F3 - Login/Logout - Assigns windows to read from far computer drives.\n\r");
	cprintf("F4 - Edit         - Executes DOS's edit on the pointer file.\n\r");
	cprintf("F5 - Copy         - Copys pointed or tagged files to other window.\n\r");
	cprintf("F6 - Move         - Moves pointed or tagged files to other window.\n\r");
	cprintf("F7 - New directory- Creates new directory.\n\r");
	cprintf("F8 - Delete       - Deletes pointed or tagged files.\n\r");
	cprintf("F9 - Menu         - Activates puuldown menus.\n\r");
	cprintf("F10- Quit         - Quits the program.\n\r");
	cprintf("                                ");
	choosebutton(1,button,0x4e,0x5f);

	delete w;
	return 0;
}


// opens the about window.
int userhelpabout()
{
	char button[][BUTTONSIZE] = {
		"OK"};
	textwin *w;

	w = new textwin(29,10,22,6,0x4e,0x4f,"About");
	if (w==NULL) return 1;

	w->show(1);


	cprintf("  BenLink v1.0\n\r");
	cprintf(" --------------\n\r");
	cprintf(" Coder: Ben hazan.\n\r");
	cprintf(" Class: JB/6.\n\r");
	cprintf(" Date : 28/6/99\n\r");



	cprintf("        ");
	choosebutton(1,button,0x4e,0x5f);

	delete w;
	return 0;
}

// configuration option. change dos path.
int userdospath()
{
	textwin *w;

	w = new textwin(2,7,76,1,0x4e,0x4f,"Enter DOS path:");
	if (w==NULL) return 1;

	w->show(1);

	getstring(DOSPATH,75);

	delete w;
	return 0;
}

// configuration option , sound on off.
int usersound()
{
	textwin *w;
	char button[][BUTTONSIZE] = {
		"OK"};

	w = new textwin(30,7,20,2,0x4e,0x4f,"Sound");
	if (w==NULL) return 1;

	w->show(1);

	if (SOUND) {
		cprintf("Sound is now OFF.\n\r");
		SOUND=0;
	} else {
		cprintf("Sound is now ON.\n\r");
		SOUND=1;
	}
	cprintf("      ");

	choosebutton(1,button,0x4e,0x5f);

	delete w;
	return 0;
}

// configuration option lpt=1/2
int userlpt()
{
	textwin *w;
	char button[][BUTTONSIZE] = {
		"OK"};

	w = new textwin(30,7,20,2,0x4e,0x4f,"LPT");
	if (w==NULL) return 1;

	w->show(1);

	if (LPTNUM) {
		cprintf("LPT port is now 1.\n\r");
		LPTNUM=0;
	} else {
		cprintf("LPT port is now 2.\n\r");
		LPTNUM=1;
	}
	cprintf("      ");

	choosebutton(1,button,0x4e,0x5f);

	delete w;
	return 0;
}

/*----------------------------- Menu options -----------------------------*/

// prints an option line -> like button.
void option(const char str[MENUSIZE],char curattr,char attr,int lighted)
{
	char t;
	int i;
	if (lighted) t=attr;
	else	     t=curattr;
	textattr(t);
	if (str[0]==NULL) {
		for (i = 0 ; i < MENUSIZE-1 ; i++)
			cprintf("Ä");
	} else cprintf("%-19s",str);
	textattr(curattr);
}

// choose from the lines. returns choosed line or menu commnad (look in defines.cpp)
int choosemenu(int x,int menunum,const char menus[][MENUSIZE])
{
	textwin *w;
	int cursor=0;
	int i;
	unsigned char k;

	w = new textwin(x,2,MENUSIZE,menunum,0x2e,0x2f,"^");
	if (w == NULL) return -1;

	w->show(1);

	for (i=0 ; i < menunum ; i++) {
		option(menus[i],0x2f,0x4c,(i==cursor));
		if (i<menunum-1) cprintf("\n\r");
	}

	do {
		k = waitkey();

		if (k==DOWN) {
			if (cursor<menunum-1) {
				do{cursor++;}while(menus[cursor][0]==NULL);
			} else {
				cursor=0;
			}
			clrscr();
			for (i=0 ; i < menunum ; i++) {
				option(menus[i],0x2f,0x4c,(i==cursor));
				if (i<menunum-1) cprintf("\n\r");
			}
		} else if (k==UP) {
			if (cursor>0) {
				do{cursor--;}while(menus[cursor][0]==NULL);
			} else {
				cursor=menunum-1;
			}
			clrscr();
			for (i=0 ; i < menunum ; i++) {
				option(menus[i],0x2f,0x4c,(i==cursor));
				if (i<menunum-1) cprintf("\n\r");
			}
		} else if ((k==PGUP)||(k==HOME)) {
			cursor=0;
			clrscr();
			for (i=0 ; i < menunum ; i++) {
				option(menus[i],0x2f,0x4c,(i==cursor));
				if (i<menunum-1) cprintf("\n\r");
			}
		} else if ((k==PGDN)||(k==END)) {
			cursor=menunum-1;
			clrscr();
			for (i=0 ; i < menunum ; i++) {
				option(menus[i],0x2f,0x4c,(i==cursor));
				if (i<menunum-1) cprintf("\n\r");
			}
		} else if ((k==ESC)||(k==F0)) {
			delete w;
			return MENUEND;
		} else if ((k==TAB)||(k==RIGHT)) {
			delete w;
			return MENURIGHT;
		} else if (k==LEFT) {
			delete w;
			return MENULEFT;
		}

	} while (k!=ENTER);

	delete w;
	return cursor;
}

// puuldown menu mode.
// prints file window and let u choose windows and lines. calls functions
// afterwards.
void usermenu(filelist *s,filelist *d)
{
	const char menu_file[][MENUSIZE] = {
		"Change drive.    F1",
		"Go to path.",
		"",
		"Edit...          F4",
		"Copy...          F5",
		"Move...          F6",
		"New directory... F7",
		"Delete...    Del/F8",
		"",
		"DOS shell.",
		"Quit to DOS.    F10"
	};

	const char menu_tag[][MENUSIZE] = {
		"Tag all.          +",
		"Tag non.          -",
		"Reverse tag.      *"
	};

	const char menu_link[][MENUSIZE] = {
		"Server mode.     F2",
		"Login/Logout.    F3"
	};

	const char menu_config[][MENUSIZE] = {
		"DOS path.",
		"Sound On/Off",
		"LPT 1/2"
	};

	const char menu_sort[][MENUSIZE] = {
		"Just directories.",
		"By name.",
		"By size.",
		"By date.",
		"By time."
	};


	const char menu_help[][MENUSIZE] = {
		"Keys.",
		"About."
	};


	int m=0,y=0;

	do {
		print_menu_line(0x3e,0x2f,m+1);
		if (m==0) {
			y = choosemenu(2,11,menu_file);
			switch(y) {
				case 0:
					userchangedrive(s);
					refresh();
					break;
				case 1:
					userpath(lisp);
					refresh();
					break;
				case 3:
					useredit(s);
					refresh();
					break;
				case 4:
					usercopy(s,d);
					updateboth(s,d);
					refresh();
					break;
				case 5:
					usermove(s,d);
					updateboth(s,d);
					refresh();
					break;
				case 6:
					usermkdir(s);
					refresh();
					break;
				case 7:
					userdelete(s);
					updateboth(s,d);
					refresh();
					break;

				case 9:
					userdosshell();
					resetlist(lis1);
					updatelist(lis1);
					resetlist(lis2);
					updatelist(lis2);
					refresh();
					break;
				case 10:
					userquit(&toexit);
					refresh();
					break;
			}
		} else
		if (m==1) {
			y = choosemenu(7,3,menu_tag);
			switch(y) {
				case 0:
					lisp->tagall();
					lisp->show();
					break;
				case 1: // remove tags.
					lisp->zerotag();
					lisp->show();
					break;
				case 2: // tag whats not tagged, untag whats tagged.
					lisp->logicalnottag();
					lisp->show();
					break;
			}
		} else
		if (m==2) {
			y = choosemenu(13,2,menu_link);
			switch(y) {
				case 0:
					server();
					resetlist(lis1);
					updatelist(lis1);
					resetlist(lis2);
					updatelist(lis2);
					refresh();
					break;
				case 1:
					log(lisp);
					refresh();
					break;
			}
		} else

		if (m==3) {
			y = choosemenu(19,5,menu_sort);
			if ((y>=0)&&(y<5)) {
				s->sortby=y;
				updatelist(s);
			}
			refresh();
		}

		if (m==4) {
			y = choosemenu(34,3,menu_config);
			switch(y) {
				case 0:
					userdospath();
					break;
				case 1: usersound();
					break;
				case 2: userlpt();
					break;
			}
		} else
		if (m==5) {
			y = choosemenu(59,2,menu_help);
			switch(y) {
				case 0:
					userhelpkeys();
					refresh();
					break;

				case 1:
					userhelpabout();
					refresh();
					break;
			}
		}


		if (y==MENURIGHT) {
			if (m<NUMOFMENUS-1) m++;
			else m=0;
		}
		if (y==MENULEFT) {
			if (m>0) m--;
			else m=NUMOFMENUS-1;
		}
		print_menu_line(0x3e,0x2f,0);

	} while ((y!=MENUEND)&&(y>=0)&&(!toexit));
}

#endif
