#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <signal.h>

char *CURRENT_DIR = NULL;
char *PATH_GS = NULL;
char *DEFAULT_PROGRAM_DIR = NULL;
char DEFAULT_DIR_FILE[] = "directory.dat";

short getArch()
{
    // SYSTEM_INFO si;
    // GetSystemInfo(&si);
    // if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL){
    //     return 32;
    // }
    // else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64){
    //     return 64;
    // }
    // else {
    //     return -1;
    // }
    return 64;
}

bool downloadGhostscript()
{
    char curl_redir[1024];
    short arch = getArch();

    printf("\n Detected architecture %hu\n", arch);

	if(arch == 32)
	{
        strcpy(curl_redir, "curl -OJL \"https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs9540/gs9540w32.exe\"");
    }
    else if(arch == 64)
	{
        strcpy(curl_redir, "curl -OJL \"https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs9540/gs9540w64.exe\"");
    }
    else
	{
        printf("ERROR_UNKNOWN_ARCHITECTURE");

        return false;
    }

    bool success = system(curl_redir);

    return !success;

}

long getFileSize(FILE *f)
{
    long pos = ftell(f);
    fseek(f, 0 , SEEK_END);
    long size = ftell(f);
    fseek(f, pos, SEEK_SET);

    return size;
}

bool installGs()
{
    system("dir /b gs*.exe > gsfilename.bat");
    bool success = !system("gsfilename.bat");

    long size = getFileSize(fopen("gsfilename.bat", "r"));
    remove("gsfilename.bat");

    if(!size)
	{
        bool s = downloadGhostscript();
        system("dir /b gs*.exe > gsfilename.bat");
        bool success = !system("gsfilename.bat");

        if(success && s)
		{
            return true;
        }
		else
		{
			return false;
		}
    }

    return success;
}

char *alocString()
{
    char tempfile[] = "+emP";
    FILE *temp = fopen (tempfile, "w+");
    char c;
    long size = 0;

    while((c = fgetc(stdin)) != '\n')
	{
        fputc(c, temp);
        size++;
    }

    rewind(temp);
    char *ret = (char *) malloc (sizeof(char) * size + 1);
    fgets(ret, sizeof(char) * size + 1, temp);
    fclose(temp);
    remove(tempfile);

    return ret;
}

bool setDefDir (char *path)
{
    FILE *defdir_file_exists = fopen(DEFAULT_DIR_FILE, "r");

    if(path)
	{
        FILE *def_dir = fopen(DEFAULT_DIR_FILE, "w");
        fputs(path, def_dir);
        fclose(def_dir);
    }

    if(defdir_file_exists == NULL)
	{
        FILE *def_dir = fopen(DEFAULT_DIR_FILE, "w");
        path = (char * )realloc(path, sizeof(char) * 20);
        strcpy (path, "C:\\Program Files\\" );
        fputs(path, def_dir);
        free (path);
        fclose(def_dir);
    }

    FILE *def_dir = fopen ( DEFAULT_DIR_FILE, "r");
    long size = getFileSize(def_dir);
    DEFAULT_PROGRAM_DIR = (char *)malloc(size);
    fgets(DEFAULT_PROGRAM_DIR, size+1, def_dir);
    fclose(def_dir);

    return true;
}

bool gsExists()
{
    setDefDir(NULL);

    DWORD dir_len = GetCurrentDirectory(0, NULL);
    CURRENT_DIR = (char *)malloc(dir_len*sizeof(char));
    GetCurrentDirectory(dir_len, CURRENT_DIR);

    char cmd[1024];
    char tempfile_gspath[] = "gs3x1.txt";
    sprintf(cmd, "cd /d  \"%s\" && dir /b /s gs*c.exe > \"%s\\%s\"", DEFAULT_PROGRAM_DIR, CURRENT_DIR, tempfile_gspath);
    bool found_gs = !system(cmd);

    if(found_gs)
	{
        FILE *o_tempfile_gspath = fopen(tempfile_gspath, "r");

        if(!o_tempfile_gspath)
		{
            printf("Failed");
        }

        fseek(o_tempfile_gspath,0,SEEK_END);
        long len = ftell(o_tempfile_gspath);
        rewind(o_tempfile_gspath);
        PATH_GS = (char *)malloc(sizeof(char)*(len));
        fgets(PATH_GS, len, o_tempfile_gspath);
        fclose(o_tempfile_gspath);
        printf("\n\nGhostscript GPL found: %s", PATH_GS );

        return true;
    }

    remove(tempfile_gspath);

    return found_gs;
}

bool init()
{
    if(gsExists())
	{
        return true;
    }
    else
	{
        printf("\nGhostscript not found in default location: %s", DEFAULT_PROGRAM_DIR);
        printf("\n\nIf it's already installed, press 'X' to set a custom default directory.");
        printf("\nPress 'Y' to install Ghostscript from the internet." );
        printf("\n\nPress 'enter' only to skip\n");
        char c[2];
        scanf("%[^\n]s", &c);
        getchar();

        if (c[0] == 'x' || c[0] == 'X')
		{
            printf("\nEnter Ghostscript directory: ");
            char *path = alocString();
            setDefDir(path);
            printf("\nPath set to %s", DEFAULT_PROGRAM_DIR);

            if(gsExists())
			{
				return true;
			}
            else
			{
                printf("\nCould not find Ghostscript in %s", DEFAULT_PROGRAM_DIR);
            }
        }
        else if(c[0] == 'Y' || c[0] == 'y')
		{
            printf("\nYou can either manually install it from 'www.ghostscript.com,'");
            printf("\n or this program can do it for you automatically.");
            printf("\n\nPress 'enter' to download and install automatically.");
            printf("\nPress Ctrl + C to exit and install manually.");
            printf("\n\nWaiting for input ...\n");
            getchar();

            if (installGs())
			{
                printf("Ghostscript installed successfully.");

                if (gsExists())
				{
                    return true;
				}
                else
				{
					return false;
				}
            }
            else
			{
                printf("Ghostscript not installed.");

                return false;
            }
        }
    }
}