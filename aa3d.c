
/*
 *          aa3d - a tool to create ascii-art stereograms
 *                  Copyright (C) 1996,1997 by
 *
 *      Jan Hubicka          (hubicka@paru.cas.cz)
 *      Thomas Marsh         (tmarsh@austin.ibm.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#define bool char

static char data[65536 / 2];

static char *default_string = "5H9NI67mfe}/ATy3\%RVjcgG{tPUbaWB(xLYosidDqkwv^]Q8FC2ZJ~zX[pK4u!nSrEMh)";

void BadTextError();

int main(int argc, char **argv)
{
    int i, x, y;	/* i, x, y are iterators  */
    int letter;		/* letter is the curent letter to draw  */
    int d, old_d;	/* d, old_d are depth for this letter and the previous one.  */
    int s;		/* s is the offset (how many characters back to match this letter to)  */
    int k;              // k is used to kill out of infinite loops
    bool l = 0, l2 = 0;	/* l, l2 are flags for when the files hit the end of a line  */
    int skip = 12;	/* skip is the length of the repeating pattern  */
    int width = 80;	/* width is the width in characters of the output  */
    char *string = NULL;/* string is the string given with the -t option  */
    bool rnd = 1;	/* rnd is a flag. if rnd is true, shift each line by a random amount  */
    int shift = 0, old_shift = 0; /* shifts the beginning of the line <shift> characters into the pattern given by the -t option  */
    bool digit = 0;	/* digit is a flag. if true, use random digits instead of letters  */
    FILE *f = NULL;

    /* Handle arguments  */
    for (i = 1; i < argc; i++) {
	if (i < argc - 1 && !strcmp(argv[i], "-w"))
	    width = atol(argv[i + 1]), i++;
	else if (i < argc - 1 && !strcmp(argv[i], "-s"))
	    skip = atol(argv[i + 1]), i++;
	else if (i < argc - 1 && !strcmp(argv[i], "-R"))
	    srand(atol(argv[i + 1])), i++;
	else if (i < argc - 1 && !strcmp(argv[i], "-t"))
	    string = argv[i + 1], i++;
	else if (i < argc - 1 && !strcmp(argv[i], "-f")) {
	    f = fopen(argv[i + 1], "r");
	    if (f == NULL)
		perror(argv[i + 1]), exit(1);
	    i++;
	} else if (!strcmp(argv[i], "-r"))
	    rnd = 0;
	else if (!strcmp(argv[i], "-d"))
	    digit = 1;
	else {
	    printf("Unknown option %s\n\n", argv[i]);
	    printf("ASCII-art stereogram generator version 1.0 by Jan Hubicka <hubicka@freesoft.cz>\n\n"
		   "Usage: %s [options] < file\n\n"
	           "File consists of spaces and digits representing depth\n\n"
		   "Options:\n"
		   "-w width   Image width (80)\n"
		   "-s step    Size of repeating seqence (12)\n"
		   "-t text    Text used to generate background\n"
		   "-r         Disable random text placement\n"
		   "-R seed    Seed random text placement\n"
		   "-f file    Use file as input for raster\n"
		   "-d         Use digits instead of letters\n"
		   ,argv[0]);
	    return 1;
	}
    }

    /* for each line in the stereogram  */
    for (y = 0; !feof(stdin); y++) {
	l = 0;
	l2 = 0;

	if (rnd && string != NULL) {
	    /* Don't let 2 lines be shifted to near the same spot (looks less random)  */
	    k = 0;
	    do{
		shift = rand() % strlen(string);
		if (k++ > 1000) BadTextError();
	    } while (strlen(string)>5 && shift - old_shift < strlen(string)/4 && old_shift - shift < strlen(string)/4);
	}

	old_shift = shift;
	old_d = 0;

	/* for each character in the line  */
	for (x = 0; x < width; x++) {

	    /* determine the depth(d), and the offset(s) of this character  */
	    if (!l && x > skip) {
		d = getc(stdin);
		if (d == ' ')
		    d = 0;
		else if (d == '\n' || d == EOF)
		    d = 0, l = 1;
		else if (d >= '0' && d <= '9')
		    d = '0' - d;
		else
		    d = -2;
	    } else
		d = 0;

	    s = d + skip;
	    s = x - s;

	    /* figure out which letter to use next  */
	    if (f != NULL) {
		if (!l2) {
		    letter = getc(f);
		    if (letter == '\n' || letter == EOF)
			letter = ' ', l2 = 1;
		} else
		letter = ' ';
	    } else {
	        k = 0;
		do{
		    letter = !digit ? default_string[rand() % strlen(default_string)] : rand() % 10 + '0';
		    if (k++ > 1000) BadTextError();
		}while (letter == data[x-1]);
	    }

	    if (string != NULL && s < 0)
		letter = string[(x + shift) % strlen(string)];

	    if (s >= 0)
		letter = data[s];

	    
	    /* if a letter falsely looks the same depth as the previous letter, 
	       then pick a new letter that doesn't  */
	    if (d > old_d && x >= skip) {
	        k = 0;
		while (letter == data[x-1] || letter == data[x-skip-old_d]) {
		    if (string != NULL && strlen(string)>2)
			letter = string[rand() % strlen(string)];
		    else
			letter = !digit ? default_string[rand() % strlen(default_string)] : rand() % 10 + '0';
		    if (k++ > 1000) BadTextError();
		}
	    }

	    /* if the last letter falsely looks the same depth as this letter,
	       then pick a new letter that doesn't  */
	    if (d < old_d && x > skip) {
	        k = 0;
		while (data[x-1] == data[x] || data[x-1] == data[x-skip-d-1]) {
		    if (string != NULL && strlen(string)>2)
			data[x-1] = string[rand() % strlen(string)];
		    else
			data[x-1] = !digit ? default_string[rand() % strlen(default_string)] : rand() % 10 + '0';
		    if (k++ > 1000) BadTextError();
		}
	    }

	    data[x] = letter;
	    old_d = d;
	}

	/* Terminate and print the line  */
	data[x]=0;
	printf("%s\n", data);

	/* finish off the current line of each input and output file  */
	s = 'a';
	while (!l && s != EOF && s != '\n')
	    s = getc(stdin);
	s = 'a';
	if (f != NULL)
	    while (!l2 && s != EOF && s != '\n')
		s = getc(f);
    }
    return 0;
}


void BadTextError() {
    printf("\nError: Your background text needs a more diverse set of characters for aa3d to work.\n\n");
    exit(1);
}

