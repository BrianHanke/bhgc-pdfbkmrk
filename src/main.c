#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <signal.h>

char *prepress;

intptr_t getLineUd(char **lineptr, size_t *n, FILE *stream)
{
    size_t pos;
    int c;

    if(lineptr == NULL || stream == NULL || n == NULL)
	{
        errno = SIGINT;

        return -1;
    }

    c = getc(stream);

    if(c == EOF)
	{
        return -1;
    }

    if(*lineptr == NULL)
	{
        *lineptr = malloc(128);

        if(*lineptr == NULL)
		{
            return -1;
        }

        *n = 128;
    }

    pos = 0;

    while(c != EOF)
	{
        if (pos + 1 >= *n)
		{
            size_t new_size = *n + (*n >> 2);

            if(new_size < 128)
			{
                new_size = 128;
            }

            char *new_ptr = realloc(*lineptr, new_size);

            if(new_ptr == NULL)
			{
                return -1;
            }

            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos ++] = c;

        if(c == '\n')
		{
            break;
        }

        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';

    return pos;
}

void reportErrors(int line)
{
	printf("ERROR: bookmarks.txt at line %d\n\n", line);
	printf("Required syntax:\n");
	printf("[Page Number] [Bookmark Name]\n\n");
	printf("HINT: Put a new line at the end if there is only one bookmark.\n");
}

int *checkBookmarks(FILE **bookmarks, int *_n_lines)
{
	int n_lines_ = 0;
	char *line = NULL;
	size_t size = 0;

	while(getLineUd(&line, &size, *bookmarks) > 0)
	{
		n_lines_++;
	}

	free(line);
	fseek(*bookmarks, 0, SEEK_SET);

	const int n_lines = n_lines_;

	int *treeArr = (int *)calloc(n_lines + 2, sizeof(int));
	{
		char *line = NULL;
		size_t size = 0;
		int i = 0,
		    temp = getLineUd(&line, &size, *bookmarks);
		bool exit_true = false;

		while(temp > 0)
		{
			if(temp == 1)
			{
				if(exit_true)
				{
					printf(", %d", i + 1);
				}
				else
				{
					printf("Please remove empty line at lines: [%d", i + 1);
				}

				exit_true = true;
			}

			int count = 0,
				sub_index = 0;

			while(line[sub_index++] == 9)
			{
				count++;
			}

			treeArr[i++] = count;
			temp = getLineUd(&line, &size, *bookmarks);
		}

		free(line);

		if(exit_true)
		{
			printf("] in bookmarks.file.\n");

			return NULL;
		}

		for(int i = 0; i < n_lines; i++)
		{
			int tabdiff = treeArr[i + 1] - treeArr[i];

			if(tabdiff > 1)
			{
				printf("Extra tabs at %d.\nUse only 1 tab per level.\n", i + 2);

				return NULL;
			}
		}
	}
	{
		for(int i = 0; i < n_lines; i++)
		{
			int j = i + 1,
			    count = 0;

			if(treeArr[j] - treeArr[i] == 1)
			{
				while(treeArr[j] - treeArr[i] >= 1)
				{
					if(treeArr[j] - treeArr[i] == 1)
					{
						count++;
					}

					j++;
				}
			}

			treeArr[i] = count;
		}
	}

	*_n_lines = n_lines;

	return treeArr;
}

bool checkFileByName(const char *filename, char code)
{
    FILE *check = fopen(filename, "r");

    if(code == 'o')
	{
        if(check)
		{
            printf("\nERROR: Output file %s already exists.\n\n", filename);
            fclose(check);

            return false;
        }
        else
		{
            return true;
        }
    }
    else
	{
        if(!check)
		{
            printf("\nERROR: Input file %s not found.\n ", filename);
            //perror(filename);

            return false;
        }
        else
		{
            fseek(check, 0, SEEK_END);

            size_t len = ftell(check);

            if(len <= 0)
			{
                printf("%s seems to be empty!\n\n ", filename);
                fclose(check);
                perror(filename);

                return false;
            }
            else
			{
                fclose(check);

                return true;
            }
        }
    }
}

bool writePostScriptFile(const char *bookmarks_name, const int n_lines, int *treeArr, short page_offset)
{
    char settings_file[] = "prepress.txt";

    prepress = (char *)malloc(strlen(settings_file));
    strcpy(prepress, settings_file);

    FILE *settings=fopen(settings_file, "w"),
         *bookmarks = fopen(bookmarks_name, "r");
    long sizelen;
    char *line = NULL,
	     *title = NULL;
    size_t size = 0;

    fseek(bookmarks, 0, SEEK_SET);
    printf("\nExecuting Ghostscript command:\n\n");

	for(int i = 0; i < n_lines; i++)
	{
		unsigned pgno;
		sizelen = getLineUd(&line, &size, bookmarks);
		title = (char*)realloc(title, sizelen * sizeof(char));

		if(!title)
		{
			fclose(settings);

			return false;
		}

		int temp = sscanf(line, "%d %[^\n]s", &pgno, title);

		if(treeArr[i])
		{
			if(temp == 2)
			{
				fprintf(settings, "[/Count %d /Page %d /Title (%s) /OUT pdfmark\n", treeArr[i], pgno + page_offset, title);
			}
			else
			{
				free(title);
				free(line);
				reportErrors(i + 1);
				fclose(settings);

				return false;
			}
		}
		else
		{
			if(temp == 2)
			{
				fprintf(settings, "[/Page %d /Title (%s) /OUT pdfmark\n", pgno + page_offset, title);
			}
			else
			{
				free(title);
				free(line);
				reportErrors(i + 1);
				fclose(settings);

				return false;
			}
		}

	}

	free(title);
	free(line);
    fclose(settings);

    return true;
}

void addBookmarks(char *output_name, char *input_name, const char *bookmarks_name)
{
    char cmd[2048];

    sprintf(cmd, "gswin64c.exe -dBATCH -dNOPAUSE -dQUIET -sDEVICE=pdfwrite -sOutputFile=%s -dPDFSETTINGS=/prepress %s %s", output_name, prepress, input_name);
    printf("%s", cmd);

    bool done = !system(cmd);

    if(done)
	{
        printf("\n\nOutput file %s saved.\n\n", output_name);
    }
	else
	{
        printf("\nError.");
    }

    remove(prepress);
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("\nERROR: Wrong number of arguments. Syntax: pdfbkmrk <input.pdf> <output.pdf>\n\n");

		return 0;
	}

    int page_offset = 0;
    char temp;
	const char *bookmarks_name = "bookmarks.txt";
	char *input_name = argv[1],
		 *output_name = argv[2];
    bool add_bookmarks = false;
    int n_lines_;
    int *treeArr;

    if(!(checkFileByName(input_name, 'i') &&
         checkFileByName(output_name, 'o') &&
         checkFileByName(bookmarks_name, 'b')))
	{
        return 0;
    }
    else
	{
        FILE *bookmarks = fopen(bookmarks_name, "r");
        treeArr = checkBookmarks(&bookmarks, &n_lines_);

        if(treeArr)
		{
            add_bookmarks = true;
        }
		else
		{
            add_bookmarks = false;
        }

        fclose(bookmarks);
    }

    if(add_bookmarks)
	{
        bool writtenPS = writePostScriptFile(bookmarks_name, n_lines_, treeArr, page_offset);

        if (writtenPS)
		{
            addBookmarks(output_name, input_name, bookmarks_name);
        }
        else
		{
            printf("\nError writing output file.");
        }

        free(treeArr);
    }
    else
	{
        printf("\n\nExiting...");
    }

    if(prepress)
	{
		free(prepress);
	}

    return 0;
}