#include "header.h"

#define MAX_MACROS 50
#define MAX_NAME 50
#define MAX_VALUE 200
#define MAX_LINE 500

typedef struct
{
	char name[MAX_NAME];
	char value[MAX_VALUE];
	char parameter[MAX_NAME];
	int is_function;
} Macro;

int is_space_char(char ch)
{
	return ch == ' ' || ch == '\t';
}

int is_identifier_start(char ch)
{
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
}

int is_identifier_char(char ch)
{
	return is_identifier_start(ch) || (ch >= '0' && ch <= '9');
}

int find_macro(Macro macros[], int macro_count, char word[])
{
	int i;

	for (i = 0; i < macro_count; i++)
	{
		if (strcmp(macros[i].name, word) == 0)
			return i;
	}

	return -1;
}

int store_macro(char line[], Macro macros[], int *macro_count)
{
	int i = 0;
	int j;
	int name_start;
	int name_length;
	int parameter_start;
	int parameter_length = 0;
	int value_start;
	int value_length;
	int position;
	int is_function = 0;
	char macro_name[MAX_NAME];
	char macro_parameter[MAX_NAME];

	while (is_space_char(line[i]))
	{
	i++;
	}

	if (line[i] != '#')
	{
	return 0;
	}

	i++;

	while (is_space_char(line[i]))
	{
	i++;
	}

	if (strncmp(&line[i], "define", 6) != 0)
	{
	return 0;
	}

	i += 6;

	if (!is_space_char(line[i]))
	{
	return 0;
	}

	while (is_space_char(line[i]))
	{
	i++;
	}

	if (!is_identifier_start(line[i]))
	{
	return 0;
	}

	name_start = i;

	while (is_identifier_char(line[i]))
	{
	i++;
	}

	name_length = i - name_start;

	if (name_length >= MAX_NAME)
	{
	printf("Macro name is too long\n");
	return 1;
	}

	for (j = 0; j < name_length; j++)
	{
	macro_name[j] = line[name_start + j];
	}

	macro_name[name_length] = '\0';

	if (line[i] == '(')
	{
		is_function = 1;
		i++;

		while (is_space_char(line[i]))
			i++;

		if (!is_identifier_start(line[i]))
			return 0;

		parameter_start = i;

		while (is_identifier_char(line[i]))
			i++;

		parameter_length = i - parameter_start;

		if (parameter_length >= MAX_NAME)
		{
			printf("Macro parameter is too long\n");
			return 1;
		}

		for (j = 0; j < parameter_length; j++)
			macro_parameter[j] = line[parameter_start + j];

		macro_parameter[parameter_length] = '\0';

		while (is_space_char(line[i]))
			i++;

		if (line[i] != ')')
			return 0;

		i++;
	}
	else
		macro_parameter[0] = '\0';

	while (is_space_char(line[i]))
		i++;

	value_start = i;

	while (line[i] != '\0' && line[i] != '\n')
		i++;

	value_length = i - value_start;

	if (value_length >= MAX_VALUE)
	{
		printf("Macro value is too long\n");
		return 1;
	}

	position = find_macro(macros, *macro_count, macro_name);

	if (position == -1)
	{
		if (*macro_count >= MAX_MACROS)
		{
			printf("Maximum macro limit reached\n");
			return 1;
		}

		position = *macro_count;
		(*macro_count)++;
	}

	strcpy(macros[position].name, macro_name);
	strcpy(macros[position].parameter, macro_parameter);

	macros[position].is_function = is_function;

	for (j = 0; j < value_length; j++)
		macros[position].value[j] = line[value_start + j];

	macros[position].value[value_length] = '\0';

	return 1;
}


void write_function_macro(FILE *opfp, Macro macro, char argument[])
{
	int i = 0;
	int start;
	int length;
	char word[MAX_NAME];

	while (macro.value[i] != '\0')
	{
		if (is_identifier_start(macro.value[i]))
		{
			start = i;

			while (is_identifier_char(macro.value[i]))
				i++;

			length = i - start;

			if (length >= MAX_NAME)
			{
				fwrite(&macro.value[start], 1, length, opfp);
				continue;
			}

			strncpy(word, &macro.value[start], length);
			word[length] = '\0';

			if (strcmp(word, macro.parameter) == 0)
				fputs(argument, opfp);
			else
				fputs(word, opfp);

			continue;
		}

		fputc(macro.value[i], opfp);
		i++;
	}
}

void replace_macros_in_line(char line[], FILE *opfp, Macro macros[], int macro_count)
{
	int i = 0;
	int j;
	int word_start;
	int word_length;
	int macro_position;
	int in_string = 0;
	int in_char = 0;
	char word[MAX_NAME];

	while (line[i] != '\0')
	{
		if (in_string == 1)
		{
			fputc(line[i], opfp);

			if (line[i] == '\\' && line[i + 1] != '\0')
			{
				i++;
				fputc(line[i], opfp);
			}
			else if (line[i] == '"')
				in_string = 0;

			i++;
			continue;
		}

		if (in_char == 1)
		{
			fputc(line[i], opfp);

			if (line[i] == '\\' && line[i + 1] != '\0')
			{
				i++;
				fputc(line[i], opfp);
			}
			else if (line[i] == '\'')
				in_char = 0;

			i++;
			continue;
		}

		if (line[i] == '"')
		{
			fputc(line[i], opfp);
			in_string = 1;
			i++;
			continue;
		}

		if (line[i] == '\'')
		{
			fputc(line[i], opfp);
			in_char = 1;
			i++;
			continue;
		}

		if (is_identifier_start(line[i]))
		{
			word_start = i;

			while (is_identifier_char(line[i]))
				i++;

			word_length = i - word_start;

			if (word_length >= MAX_NAME)
			{
				fwrite(&line[word_start], 1, word_length, opfp);
				continue;
			}

			for (j = 0; j < word_length; j++)
				word[j] = line[word_start + j];

			word[word_length] = '\0';

			macro_position = find_macro(macros, macro_count, word);

			if (macro_position == -1)
				fputs(word, opfp);
			else if (macros[macro_position].is_function == 0)
				fputs(macros[macro_position].value, opfp);
			else
			{
				int k = i;
				int argument_start;
				int argument_length;
				int bracket_count = 1;
				char argument[MAX_VALUE];

				while (is_space_char(line[k]))
					k++;

				if (line[k] != '(')
				{
					fputs(word, opfp);
					continue;
				}

				k++;
				argument_start = k;

				while (line[k] != '\0' && bracket_count > 0)
				{
					if (line[k] == '(')
						bracket_count++;
					else if (line[k] == ')')
						bracket_count--;

					if (bracket_count > 0)
						k++;
				}

				if (line[k] != ')')
				{
				fputs(word, opfp);
				continue;
				}

				argument_length = k - argument_start;

				if (argument_length >= MAX_VALUE)
				{
				fputs(word, opfp);
				continue;
				}

				strncpy(argument, &line[argument_start], argument_length);
				argument[argument_length] = '\0';

				write_function_macro(opfp, macros[macro_position], argument);

				i = k + 1;
			}

			continue;
		}

		fputc(line[i], opfp);
		i++;
	}
}


void macro_substitution(FILE *ipfp, FILE *opfp)
{
    Macro macros[MAX_MACROS];
    int macro_count = 0;
    char line[MAX_LINE];
    int i;

    while (fgets(line, sizeof(line), ipfp) != NULL)
    {
        i = 0;

        while (line[i] == ' ' || line[i] == '\t')
        {
            i++;
        }

        /*
         * Keep every preprocessor directive unchanged.
         * This protects #ifndef, #define, #if, #endif, etc.
         */
        if (line[i] == '#')
        {
            /*
             * Still store simple user macros such as:
             * #define PI 3.14
             */
            store_macro(line, macros, &macro_count);

            /* Copy the directive line unchanged */
            fputs(line, opfp);
        }
        else
        {
            /* Replace macros only in normal C-code lines */
            replace_macros_in_line(line, opfp, macros, macro_count);
        }
    }
}
