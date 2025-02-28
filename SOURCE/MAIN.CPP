/*-------------------------------------------------------------------------*\
|			M	A	I	N			    |
\*-------------------------------------------------------------------------*/

#include <init.cpp>
/*- This one ^ has most of the include files and functions in it. -*/
#include <function.cpp>
/*- This ine ^ has the program functions in it. -*/


void main(int argn,char *argv[])
{
 /*----------------------- Variables ---------------------------*/

	unsigned char k; // Uses for getch.
	int i; // uses for return from far functions.

 /*----------------------- Initializing ------------------------*/

	/*----- From here: is the part of the setup program which creat the Benlink.cfg -*/

	if (argn > 1) {
		if (argn == 2) {
			strupr(argv[1]);
			if (strcmp(argv[1],"/D")==0)
				creatdefaultconfig();
			else {
				cprintf("Syntax: Benlink [/d]\n\r");
				cprintf("/d: This option will creat the default configuration file.\n\r");
				cprintf("    You should run it before you run the benlink program.\n\r");
			}
		}
		return;
	}

	/*----- Till here -*/

	init(); // declared in init.cpp initializes memory and shows init screen.
	// also loads the configuration. has the global vars.

 /*------------------------------ MAIN LOOP --------------------------*/

	do {
		k=waitkey();	      // Waits till a key is pressed.
		switch (k) {

			case UP : lisp->movepos(-1);break;
			case DOWN : lisp->movepos(1);break;
			case PGUP : lisp->movepos(-FILE_WINS_LINES+3);break;
			case PGDN : lisp->movepos(FILE_WINS_LINES-3);break;
			case HOME : lisp->home();lisp->show();break;
			case END  : lisp->end();lisp->show();break;
				// If arrows, then change position of list
				// cursor.

			       /*----------------------------*/

			// switch window.
			case TAB :case RIGHT:case LEFT :
				winp->show(0);
				lisp->show();
				lisp->removecursor();
				if (lisp==lis1)lisp=lis2;  //
				else lisp=lis1;            // switch "active" pointers
				if (winp==win1)winp=win2;  // and refresh.
				else winp=win1;            //
				if (lisp->farcomp) {
					if (farsetdisk(lisp->drive,&i)) {
						msgbox("Error","Could not recive far data. Reading local drive.");
						resetlist(lisp);
						lisp->zerotag();
						lisp->home();
					} else {
						if (farchdir(lisp->path,&i)) {
							msgbox("Error","Could not receive far data. Reading local directory");
							resetlist(lisp);
							lisp->zerotag();
							lisp->home();
						}
					}
				} else {
					setdisk(lisp->drive);
					chdir(lisp->path);
				}
				//if (updatelist(lisp)) {
				//	msgbox("Error","Could not receive far data. Reading local file list.");
				//}
				refresh();
				break;


			      /*----------------------------*/

			// Tagging...
			case SPACE:case INS: lisp->tag();break;
				// If space, tag file under cursor.
			case '+': // Tag all files in list.
				lisp->tagall();
				lisp->show();
				break;
			case '-': // remove tags.
				lisp->zerotag();
				lisp->show();
				break;
			case '*': // tag whats not tagged, untag whats tagged.
				lisp->logicalnottag();
				lisp->show();
				break;

			      /*--------- Functions ----------*/

			// changing directory.
			case ENTER:
				userchangedirectory(lisp);   // enter the directory and scan it.
				refresh(); // redraw windows.
				break;
			case F1 :
				userchangedrive(lisp); // change drive, scan it.
				refresh();
				break;
			case F2 :
				server(); 	// enter server mode.
				resetlist(lis1);	// rescan and resfresh the filelists afterwards.
				updatelist(lis1);
				resetlist(lis2);
				updatelist(lis2);
				refresh();
				break;
			case F3 :
				log(lisp);	// Logs in and out the
				refresh();	// server computer.
				break;
			case F4 :
				useredit(lisp); // spawns edit.
				refresh();
				break;
			case F5 :
				if (lisp==lis1) usercopy(lis1,lis2);
				else 		usercopy(lis2,lis1);
				if (lisp==lis1) updateboth(lis1,lis2);
				else 		updateboth(lis2,lis1);
				refresh();
				// copys from source to destination.
				// update both lists.
				// refresh windows.
				break;
			case F6 :
				if (lisp==lis1) usermove(lis1,lis2);
				else 		usermove(lis2,lis1);
				// moves.
				if (lisp==lis1) updateboth(lis1,lis2);
				else 		updateboth(lis2,lis1);
				refresh();
				break;
			case F7 :
				usermkdir(lisp);
				// makes new directory. and scans ofcourse.
				refresh();
				break;
			case F8 : case DEL:
				userdelete(lisp);
				if (lisp==lis1) updateboth(lis1,lis2);
				else	        updateboth(lis2,lis1);
				refresh();
				// deletes filelist.
				break;
			case F9 : case ESC:
				if (lisp==lis1) usermenu(lis1,lis2);
				else 		usermenu(lis2,lis1);
				refresh();
				// enter pulldown menus mode.
				break;
			case F0:
				userquit(&toexit);
				// prompts for quitting.
				refresh();
				break;
		}
	} while (!toexit);

 /*----------------------------- The end ----------------------------*/

	shutdown();
}


