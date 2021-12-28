
#ifndef __DICE_H
#define __DICE_H

typedef struct diceroll
{

	char name[40];
	char monster[50];
	char prefix[40];
	int roll;
	int rollnum;
	int sides;
	int rollmax;

	D3DVERTEX dicebox[4];
};

extern diceroll dice[50];
extern int numdice;
extern int showsavingthrow;
#endif 