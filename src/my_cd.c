#define _XOPEN_SOURCE 700 // C'est de la magie noire.

#include "utils.h"
#include "lib.h"

int my_cd(char* cur, char** query){
    char option = 'D';
    int has_opt = 0;
    int argc = argvlen(query);

    char* directory = calloc(MAX_STRING_LENGTH, sizeof(char));

    if (argc == 0){strcpy(directory, "~");}
    else {
        if (strcmp(query[0], "-L") == 0 || strcmp(query[0], "-P") == 0){
            option = query[0][1];
            has_opt++;
        }
        if (argc > has_opt + 1){
            write(1, "cd: too many arguments\n", strlen("cd: too many arguments\n"));
            return 1;
        }
        if (argc == has_opt){strcpy(directory, "~");}
        else {strcpy(directory, query[has_opt]);}
    }

    // Les étapes ci-dessous sont celles explicités dans le manuel.
    int found_file = 0;

    // Étapes 1-2
    if (strcmp(directory, "~") == 0){
        // Pas de requête, on tente de rejoindre le home.
        char* curpath = getenv("HOME");
        if (curpath && setenv("OLDPWD", cur, 1) == 0 && setenv("PWD", curpath, 1) == 0){
            return 0;
        }
        return 1;
    }

    if (strcmp(directory, "-") == 0){
        // Go to previous directory
        char* curpath = getenv("OLDPWD");
        if (curpath && setenv("OLDPWD", cur, 1) == 0 && setenv("PWD", curpath, 1) == 0){
            write(1, curpath, strlen(curpath));
            write(1, "\n", 1);
            return 0;
        }
        return 1;
    }
    // Initialisation de curpath utile pour les prochaines étapes.
    char* curpath = calloc(MAX_STRING_LENGTH, sizeof(char));

    // Étape 3
    if (directory && directory[0] == '/'){
        strcpy(curpath, directory);
        goto etape7;
    }

    // Étape 4
    if (directory && directory[0] == '.'){
        goto etape6;
    }

    // Étape 5
    char* CDPATH = calloc(MAX_STRING_LENGTH, sizeof(char));
    strcpy(CDPATH, cur);

    // Ajout du chemin actuel
    if (getenv("CDPATH")){
        strcat(CDPATH, ":");
        strcat(CDPATH, getenv("CDPATH"));
    }

    // On itère dans CDPATH
    struct stat sb;
    char *token = strtok(CDPATH, ":");

    while (token != NULL) {
        char* np = calloc(MAX_STRING_LENGTH, sizeof(char));
        if(!np){free(CDPATH);return 1;}
        strcpy(np, token);

        if (np[strlen(np)-1] != '/'){
            strcat(np, "/");
        }

        strcat(np, directory);
        if (stat(np, &sb) == 0){
            if ((sb.st_mode & S_IFMT) == S_IFDIR){
                // On a trouvé le bon dossier
                strcpy(curpath, np);

                free(np);
                free(CDPATH);
                goto etape7;
            } else {
                // On a trouvé un fichier avec ce nom
                found_file = 1;
            }
        }
        if (np){free(np);}
        token = strtok(NULL, ":");
    }
    free(CDPATH);

    // Étape 6
    etape6:
    strcpy(curpath, directory);

    // Étape 7
    etape7:
    if (curpath[0] != '/'){
        char* temp = calloc(MAX_STRING_LENGTH, sizeof(char));
        if(!temp){free(curpath);return 1;}
        strcat(strcat(strcat(temp, cur), "/"), curpath);
        strcpy(curpath, temp);
        free(temp);
    }

    // Étape 8-9 TODO
    if (option == 'L'){
        //TODO
    }

    // Étape 10
    if (found_file){
        free(curpath);
        write(1, "cd: ", 4);
        write(1, directory, strlen(directory));
        write(1, " : Not a directory\n", strlen(" : Not a directory\n"));
        return 1;
    }

    char* _curpath = calloc(MAX_STRING_LENGTH, sizeof(char));
    if (realpath(curpath, _curpath)){
        if (setenv("OLDPWD", cur, 1) == 0 && setenv("PWD", _curpath, 1) == 0){
            free(curpath);
            free(_curpath);
            return 0;
        }
    }

    free(_curpath);
    free(curpath);
    write(1, "cd: ", 4);
    write(1, directory, strlen(directory));
    write(1, " : No such file or directory\n", strlen(" : No such file or directory\n"));
    return 1;
}