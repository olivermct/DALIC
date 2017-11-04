/* indexer.c	Oliver McTammany	CS50
 *
 * takes input from a directory that was created by crawler and 
 * indexes the words into an output file in the form
 * word ID freq
 * where ID is the number of the document in crawler and freq is the frequency that the word appears in that dacument
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "word.h"
#include "webpage.h"
#include "index.h"

static int checkParam(int argc, char* argv[]);
int index(char* pagedir, char* filename);
index_t *build_index(char *pagedir, index_t *index);
webpage_t* make_webpage(FILE *fp);
void clean_file(char *oldFile, char *newFile);

/*************** main *******************/
/* checks parameters and calls index
 * returns int based on exit status */
int main(int argc, char* argv[])
{
	int status = checkParam(argc, argv);
	if (status != 0) {
		return status;
	}

	char* pagedir = argv[1];
	char* filename = argv[2];

	index(pagedir, filename);
}

/************** checkParam *************/
/* checks parameters argc and argv
   argc must be equal to 3
   argv[1] must be an openable directory
   argv[2] just has to be a file that may or may not exists, so the 
     argument will always be okay since we can just make a new file
   returns 0 if success, error code if failure  */
int checkParam(int argc, char* argv[])
{
	if (argc > 3) {
		fprintf(stderr, "Too many arguments\n");
		return 1;
	}
	if (argc < 3) {
		fprintf(stderr, "Too few arguments\n");
		return 2;
	}

	DIR *dir = opendir(argv[1]);
	if (!dir) {
		fprintf(stderr, "Invalid directory\n");
		return 3;
	} closedir(dir);
	return 0;
}

/***************** index *********************/
/* calls functions to create and save an index
   pagedir is the directory that comes from crawler
   filename is where the index will be saved after it is created */
int index(char* pagedir, char* filename)
{
	index_t *index = index_new(0);
	if (index == NULL) {
		fprintf(stderr, "error creating index\n");
		return 4;
	}
	index = build_index(pagedir, index);
	if (index == NULL) {
		return 5;
	}
	index_save(index, "temp");
	clean_file("temp", filename);

	index_delete(index);

	return 0;
}

/*************** build_index ********************/
/* builds an index object with all the information from the input file
   pagedir is the output from crawler that needs to be indexed
   index is the target where the information will be stored
   returns the pointer to the index */
index_t* build_index(char* pagedir, index_t* index)
{
	int ID = 1;
	char pathFile[500];													// pathFile assumed to be less than 500 characters
	sprintf(pathFile, "%s/%d", pagedir, ID);

	FILE *fp = NULL;
	webpage_t *page;

	while ((fp = fopen(pathFile, "r")) != NULL) {						// file is closed in make_webpage()
		page = make_webpage(fp);										// extract information from crawler page
		if (page == NULL) {
			fprintf(stderr, "error making webpage\n");
			return NULL;
		}

		char* word;
		int pos = 0;
		while (((pos = webpage_getNextWord(page, pos, &word)) > 0)) {
			char* normal = NormalizeWord(word);
			if (!index_insert(index, normal, ID, 1)) {
				fprintf(stderr, "error inserting into index\n");
				return NULL;
			}
		}

		webpage_delete(page);
		
		ID++;
		sprintf(pathFile, "%s/%d", pagedir, ID);						// iterate to next file
	}

	return index;
}

/******************** make_webage *****************/
/* creates the webpage to be used by build index
   takes the html ID file and turns it back into a webpage
   fp must be the file containing the URL, depth, and html */
webpage_t *make_webpage(FILE *fp)
{
	char URL[1000] = "";											// URL assumed to be no more than 1000 chars because I hate malloc
	fseek(fp, 0L, SEEK_END);										// get the size of the file because size of html will be less thn that
	int size = ftell(fp);
	char* html = malloc(size);
	rewind(fp);														// reset file to read characters
	
	char c;
	while ((c = fgetc(fp)) != '\n') {
		int len = strlen(URL);
		URL[len] = c;
		URL[len+1] = '\0';
	}

	int depth = (int)fgetc(fp) - 48;								// subtract 48 because that yields the right numbers

	while ((c = fgetc(fp)) != EOF) {
		int len = strlen(html);
		html[len] = c;
		html[len+1] = '\0';	
	}
	fclose(fp);

	return webpage_new(URL, depth, html);
}


/****************** clean_file *********************/
/* since we're using the library hashtable, it prints with a lot of junk
   this cleans the junk to get the output file in the right format
   oldFile is the temporary file created by index
   newFile is the actual output file */
void clean_file(char *oldFile, char *newFile)
{
	FILE *ofp = fopen(oldFile, "r");
	FILE *nfp = fopen(newFile, "w");
	char c;
	while ((c = fgetc(ofp)) != EOF) {
		if (c != '{' && c != '}') {
			fputc(c, nfp);
		}
	}

	fclose(ofp);
	fclose(nfp);

	remove(oldFile);							// the old file is not needed, so delete it
}