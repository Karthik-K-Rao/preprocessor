#include "header.h"

#define MAX_INCLUDED_FILES 500
#define MAX_PATH_LENGTH 1024

static char included_files[MAX_INCLUDED_FILES][MAX_PATH_LENGTH];
static int included_count = 0;
static int recursion_depth = 0;

int already_included(char path[])
{
    int i;

    for (i = 0; i < included_count; i++)
    {
        if (strcmp(included_files[i], path) == 0)
        {
            return 1;
        }
    }

    return 0;
}

void remember_header(char path[])
{
    if (included_count < MAX_INCLUDED_FILES)
    {
        strcpy(included_files[included_count], path);
        included_count++;
    }
}

FILE *try_header_path(char directory[], char header_name[], char full_path[])
{
    FILE *hfp;

    snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", directory, header_name);

    hfp = fopen(full_path, "r");

    return hfp;
}

FILE *open_system_header(char header_name[], char full_path[])
{
    FILE *hfp;
    FILE *gcc_pipe;
    char gcc_include_path[MAX_PATH_LENGTH];
    char *system_paths[] =
    {
        "/usr/local/include",
        "/usr/include/x86_64-linux-gnu",
        "/usr/include"
    };
    int i;

    /* First, allow a local <values.h> header */
    hfp = fopen(header_name, "r");

    if (hfp != NULL)
    {
        strcpy(full_path, header_name);
        return hfp;
    }

    /* Ask GCC for its private header directory */
    gcc_pipe = popen("gcc -print-file-name=include", "r");

    if (gcc_pipe != NULL)
    {
        if (fgets(gcc_include_path, sizeof(gcc_include_path), gcc_pipe) != NULL)
        {
            gcc_include_path[strcspn(gcc_include_path, "\n")] = '\0';

            hfp = try_header_path(gcc_include_path, header_name, full_path);

            if (hfp != NULL)
            {
                pclose(gcc_pipe);
                return hfp;
            }
        }

        pclose(gcc_pipe);
    }

    /* Common Ubuntu system-header locations */
    for (i = 0; i < 3; i++)
    {
        hfp = try_header_path(system_paths[i], header_name, full_path);

        if (hfp != NULL)
        {
            return hfp;
        }
    }

    return NULL;
}

void header_inclusion(FILE *ipfp, FILE *opfp)
{
    char line[256];
    char header_name[256];
    char full_path[MAX_PATH_LENGTH];
    int i;
    int j;
    int is_system_header;
    FILE *hfp;
    FILE *temp_header;

    if (recursion_depth == 0)
    {
        included_count = 0;
    }

    recursion_depth++;

    while (fgets(line, sizeof(line), ipfp) != NULL)
    {
        i = 0;

        while (line[i] == ' ' || line[i] == '\t')
        {
            i++;
        }

        if (line[i] != '#')
        {
            fputs(line, opfp);
            continue;
        }

        i++;

        while (line[i] == ' ' || line[i] == '\t')
        {
            i++;
        }

        if (strncmp(&line[i], "include", 7) != 0)
        {
            fputs(line, opfp);
            continue;
        }

        i += 7;

        while (line[i] == ' ' || line[i] == '\t')
        {
            i++;
        }

        if (line[i] == '<')
        {
            is_system_header = 1;
        }
        else if (line[i] == '"')
        {
            is_system_header = 0;
        }
        else
        {
            fputs(line, opfp);
            continue;
        }

        i++;
        j = 0;

        while (line[i] != (is_system_header ? '>' : '"') &&
               line[i] != '\0' &&
               line[i] != '\n' &&
               j < 255)
        {
            header_name[j] = line[i];
            i++;
            j++;
        }

        header_name[j] = '\0';

        if (line[i] != (is_system_header ? '>' : '"'))
        {
            printf("Invalid include line: %s", line);
            continue;
        }

        if (is_system_header == 1)
        {
            hfp = open_system_header(header_name, full_path);
        }
        else
        {
            hfp = fopen(header_name, "r");

            if (hfp != NULL)
            {
                strcpy(full_path, header_name);
            }
        }

        if (hfp == NULL)
        {
            printf("Cannot open header: %s\n", header_name);

            /* Keep an unresolved system include in the output */
            if (is_system_header == 1)
            {
                fputs(line, opfp);
            }

            continue;
        }

        /* Prevent repeated/circular header expansion */
        if (already_included(full_path))
        {
            fclose(hfp);
            continue;
        }

        remember_header(full_path);

        printf("Including: %s\n", full_path);

        temp_header = tmpfile();

        if (temp_header == NULL)
        {
            printf("Cannot create temporary file for: %s\n", header_name);
            fclose(hfp);
            continue;
        }

        /* Remove comments from the header */
        comment_removal(hfp, temp_header);
        rewind(temp_header);

        /* Expand nested headers */
        header_inclusion(temp_header, opfp);

        fclose(temp_header);
        fclose(hfp);
    }

    recursion_depth--;
}
