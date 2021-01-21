#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAX_VARS 100
#define TRUE 1
#define FALSE 0

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
	 
	 
/* FUNCTION VARIABLES */
char *funcNames[MAX_VARS];
int numFuncs = 0;

void addFuncName(char * func) {
	funcNames[numFuncs] = func;
	numFuncs++;
}

int isFunc(char * name) {
	if (strcmp("clock", name) == 0) { return TRUE; }
	for (int i = 0; i < numFuncs; i++) {
		if (strcmp(funcNames[i], name) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}
	 
/* INTEGER VARIABLES */
char *intNames[MAX_VARS];
int numInts = 0;

void addInt(char * name) {
	intNames[numInts] = malloc(sizeof(intNames[numInts]));
	strcpy(intNames[numInts], name);
	numInts++;
}

int getIntOffset(char * name) {
	for (int i = 0; i < numInts; i++) {
		if (strcmp(intNames[i], name) == 0) {
			return 8*(i+1); // end of array is 0, start is max
		}
	}
	return -1;
}

/* STRING VARIABLES */

char *strVarNames[MAX_VARS];
int strLocs[MAX_VARS];
int numStrVars = 0;

void addStrVar(char * name, int l) {
	strVarNames[numStrVars] = name;
	strLocs[numStrVars] = l;
	numStrVars++;
}

int getStrVar(char * name) {
	for (int i = 0; i < numStrVars; i++) {
		if (memcmp(strVarNames[i], name, strlen(name)) == 0) {
			return strLocs[i]; // end of array is 0, start is max
		}
	}
	return -1;
}

/* STATIC STRINGS */

int EAX = 0;
int EBX = 0;
int ECX = 0;
int EDX = 0;

char * nextRegister() {
	if (!EAX) {EAX=1;return "%rax";}
	if (!EBX) {EBX=1;return "%rbx";}
	if (!ECX) {ECX=1;return "%rcx";}
	if (!EDX) {EDX=1;return "%rdx";}
	return 0;
}

void freeRegister(char * reg) {
	if (strcmp(reg, "%rax") == 0) {
		EAX = 0;
	} else if (strcmp(reg, "%rbx") == 0) {
		EBX = 0;
	} else if (strcmp(reg, "%rcx") == 0) {
		ECX = 0;
	} else if (strcmp(reg, "%rdx") == 0) {
		EDX = 0;
	}
}

char stringNames[MAX_VARS][100];
int numStrings = 0;

int addString(char * string) {
	printf("ADDING %s\n", string);
	printf("ADDED %s\n", string);
	strcpy(stringNames[numStrings], string);
	printf("ADDED %s\n", string);
	numStrings++;
	return numStrings-1;
}

int getStringLoc(char * name) {
	for (int i = 0; i < numStrings; i++) {
		if (strcmp(name, stringNames[i]) == 0) {
			return i;
		}
	}
	return -1;
}

int jumpLoc = 0;
int newJumpLoc() {
	jumpLoc++;
	return jumpLoc;
}
