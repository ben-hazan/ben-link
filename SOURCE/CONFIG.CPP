#ifndef __CONFIG_CPP
#define __CONFIG_CPP

#ifndef __STDIO_H
#include <stdio.h>
#endif

#ifndef __CONIO_H
#include <conio.h>
#endif

#ifndef __DIR_H
#include <dir.h>
#endif

#ifndef __DEFINES_CPP
#include <defines.cpp>
#endif

int SOUND =1;
int LPTNUM = 0;
char DOSPATH[MAXPATH] = "c:\\dos\\";
// main global vars which use the configurator.


int creatdefaultconfig()
{
// this function opens a new benlink.cfg file and writes the default
// configuration to it.

	char buf[256];
	FILE *tofile;

	if ((tofile = fopen("benlink.cfg","wt"))==NULL) {
	cprintf("Error creating configuration file!\n\r");
		asm xor ax,ax
		asm int 16h
		return 1;
	}
	fprintf(tofile,"SOUND=YES\n");
	fprintf(tofile,"LPT=1\n");
	fprintf(tofile,"DOS=C:\\DOS\\\n");
	fprintf(tofile,"; This is the default configuration file. you should edit it to your\n;settings. It will be load any time you run BenLink.\n;");
	fprintf(tofile,"The options for the sound variable are 'yes' and 'no'. exp: 'sound=no' ->\n; this means that there will be no sound\n;");
	fprintf(tofile,"The options for the LPT variable are 1 and 2.\n;");
	fprintf(tofile,"In the DOS field you should write the dos path with a '\\' at the end.\n;");
	fprintf(tofile,"Exp: dos=c:\\windows\\command\\ <== (see the '\\'?)");

	fclose(tofile);

	cprintf("Default config file (benlink.cfg) has been created.\n\r");
	cprintf("Please don't forget to edit it.\n\r");
	return 0;
}

int loadconfig()
{
// this function opens the benlink.cfg and reads the config from there.
// prompts in case of error. updates the global vars which use config.

	FILE *tofile;
	char buf[256];

	if ((tofile = fopen("benlink.cfg","rt"))==NULL) {
		return 4;
	}

	fscanf(tofile,"%s",buf);
	strupr(buf);
	if (strcmp("SOUND=NO",buf)==0) {
		SOUND =0;
	} else {
		if (strcmp("SOUND=YES",buf)==0) {
			SOUND =1;
		} else {
			return 1;
		}
	}

	fscanf(tofile,"%s",buf);
	strupr(buf);
	if (strcmp("LPT=1",buf)==0) LPTNUM=0;
	else if (strcmp("LPT=2",buf)==0) LPTNUM=1;
	else {
		return 2;
	}

	fscanf(tofile,"%s",buf);
	strupr(buf);
	if (strncmp("DOS=",buf,4)==0) strcpy(DOSPATH,buf+4);
	else {
		return 3;
	}
	fclose(tofile);

	return 0;
}

#endif







