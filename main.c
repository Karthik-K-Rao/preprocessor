#include "header.h"

int main(int ac, char **av)
{
	FILE *ipfp;
	FILE *opfp;
	FILE *tempfp;
	FILE *tempfp2;
	int length;
	char output_file[256];

	if (ac != 2)
	{
		printf("\033[1;31mUsage: ./mypreprocessor inputfile.c\033[0m\n");
		return 1;
	}

	length = strlen(av[1]);

	if (length < 3 || strcmp(av[1] + length - 2, ".c") != 0)
	{
		printf("\033[1;31mInput file must have .c extension\033[0m\n");
		return 1;
	}

	if (length >= 256)
	{
		printf("\033[1;31mFilename is too long\033[0m\n");
		return 1;
	}

	ipfp = fopen(av[1], "r");

	if (ipfp == NULL)
	{
		printf("\033[1;31mCannot open file: %s\033[0m\n", av[1]);
		return 1;
	}

	strcpy(output_file, av[1]);
	output_file[length - 1] = 'i';

	opfp = fopen(output_file, "w");

	if (opfp == NULL)
	{
		printf("\033[1;31mCannot create file: %s\033[0m\n", output_file);
		fclose(ipfp);
		return 1;
	}

	tempfp = tmpfile();

	if (tempfp == NULL)
	{
		printf("\033[1;31mCannot create temporary file\033[0m\n");
		fclose(ipfp);
		fclose(opfp);
		return 1;
	}
	tempfp2 = tmpfile();

	if (tempfp2 == NULL)
	{
		printf("\033[1;31mCannot create temporary file\033[0m\n");
		fclose(ipfp);
		fclose(tempfp);
		fclose(opfp);
		return 1;
	}

	printf("Input file : %s\n", av[1]);
	printf("Output file: %s\n", output_file);

//STEP 1: REMOVE COMMENTS FROM THE SOURCE FILE
	comment_removal(ipfp, tempfp);

//STEP 2: READ THE COMMENT-FREE TEMPORARY FILE
	rewind(tempfp);

//STEP 3: REPLACE #include LINES
	header_inclusion(tempfp, tempfp2);

//STEP 4: READ INCLUDE-PROCESSED TEMPORARY FILE
	rewind(tempfp2);

//STEP 5: REPLACE SIMPLE MACROS IN FINAL OUTPUT
	macro_substitution(tempfp2, opfp);

	fclose(ipfp);
	fclose(tempfp);
	fclose(tempfp2);
	fclose(opfp);

	return 0;
}
