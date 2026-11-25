#include "header.h"

void comment_removal(FILE *ipfp, FILE *opfp)
{
	int ch;
	int next;
	int previous;
	int in_string = 0;
	int in_char = 0;
	while ((ch = fgetc(ipfp)) != EOF)
	{
		if (in_string == 1)
		{
			fputc(ch, opfp);
			if (ch == '\\')
		 	{
		 		ch = fgetc(ipfp);

		        	if (ch != EOF)
		            		fputc(ch, opfp);
		        	continue;
		    	}
		    	if (ch == '"')
			        in_string = 0;
		    	continue;
		}

		if (in_char == 1)
		{
			fputc(ch, opfp);

		    	if (ch == '\\')
		    	{
		        	ch = fgetc(ipfp);

				if (ch != EOF)
				    fputc(ch, opfp);
		        	continue;
		    	}

			    if (ch == '\'')
				in_char = 0;

		    	continue;
		}

		if (ch == '"')
		{
			fputc(ch, opfp);
			in_string = 1;
			continue;
		}

		if (ch == '\'')
		{
			fputc(ch, opfp);
			in_char = 1;
			continue;
		}

		if (ch == '/')
		{
			next = fgetc(ipfp);
//SINGLE LINE COMMENTS
		 	if (next == '/')
		    	{
		        	while ((ch = fgetc(ipfp)) != '\n' && ch != EOF);

		        	if (ch == '\n')
		            		fputc('\n', opfp);
		    	}
//MULTI LINE COMMENTS
		    	else if (next == '*')
		    	{
		        	previous = 0;

		        	while ((ch = fgetc(ipfp)) != EOF)
		        	{
		            		if (ch == '\n')
		                		fputc('\n', opfp);

		            		if (previous == '*' && ch == '/')
		                		break;

		            	previous = ch;
		        	}
		    	}

// A NORMAL'/'
		    	else
		    	{
		        	fputc('/', opfp);

		        	if (next != EOF)
		            		fputc(next, opfp);
		    	}
		}

// ORDINARY CHARECTERS
	       	else
		    fputc(ch, opfp);
    	}
}
