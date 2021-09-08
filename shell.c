
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "shell.h"

const char* valid_builtin_commands[] = {"cd", "exit", NULL};


void parse( char* line, command_t* p_cmd ) {

//    int i = 0;
//    int count = 0;
//    char ch = line[0];
//    while (ch != '\0') {
//        ch = line[i];
//        if (ch == 32){count++;}
//
//        i++;
//    }
//    if (count == 0){
//        line[strlen(line)-1] = '\0';
//    }

    //line[strlen(line)-1] = '\0';
    // TO-DO: COMPLETE THIS FUNCTION BODY
    p_cmd->path = malloc(sizeof(char)*100);
    p_cmd->argv = malloc(sizeof(char*)*50);
    for ( int i=0; i<50; i++ ) p_cmd->argv[i] = malloc(sizeof(char)*100);
    if (line[0] == '\0') { //EMPTY INPUT
        p_cmd->argc = 0;
        p_cmd->argv = NULL;
        p_cmd->path = NULL;
        //printf("%d\n",p_cmd->argc); printf("%s\n", p_cmd->path); printf("%s\n", p_cmd->argv);
        return;
    }
    //printf("%d",line[strlen(line)-1]); //**********the last char is always new line, not \0***********
    char* buf; //TO STORE THE PARSED PORTION OF INPUT
    int argc = 0; //ARGC COUNTER

    buf = strtok(line, " ");
    strcpy(p_cmd->path,buf);
    //strcpy(p_cmd->argv[0],buf);
    //p_cmd->path[strlen(p_cmd->path)-1] = '\0';
    while (buf != NULL) {
        strcpy(p_cmd->argv[argc],buf);
        buf = strtok(NULL," ");
        argc++;
    }
    
    p_cmd->argc = argc;
    p_cmd->argv[argc] = NULL;


    if (find_fullpath(p_cmd->path,p_cmd)) { //CHECK IF PATH EXISTS IN BUILTIN FOLDERS

        return;
    }
    else { //FAKE COMMAND
        p_cmd->argc = ERROR;
        p_cmd->argv = NULL;
    }



// end parse function
    }



int find_fullpath( char* command_name, command_t* p_cmd ) {
    // TO-DO: COMPLETE THIS FUNCTION BODY
    char * path = strdup(getenv("PATH"));
    //printf("%s",path);

    char ** path_table = malloc(sizeof(char*) * 100);
    for (int i=0; i<50; i++) {
        path_table[i] = malloc(sizeof(char) * 50);
    }
    char* buf = strtok(path,":");
    strcpy(path_table[0],"/");
    int i = 0;
    while (buf != NULL) {
        strcpy(path_table[i],buf);
        buf = strtok(NULL,":");
        i++;
    }

    //write(1,p_cmd->path,strlen(p_cmd->path));
    int builtin = FALSE;

    if (equals(command_name, "exit")){
        return TRUE;
        //builtin = TRUE;
    }
    //write(1,"w r",strlen("w r"));
    if (equals(command_name, "cd")){
        return TRUE;
        //builtin = TRUE;
    }
    char * executePath = NULL;
    struct stat stats;
    char * buffer = malloc(sizeof(char) * 100);
    for (int j = 0; path_table[j]; j++) {
        strcpy(buffer, path_table[j]);
        strcat(buffer, "/");
        strcat(buffer, command_name);
        if (stat(buffer, & stats) == 0) {
            executePath = buffer;
            break;
        }
    }

    if (!executePath) { // failure
        //return FALSE;
        builtin = FALSE;
    } else {
        strcpy(p_cmd->path,executePath);
        //return TRUE;
        builtin = TRUE;
    }

    return builtin;

} // end find_fullpath function


int execute( command_t* p_cmd ) {
    // TO-DO: COMPLETE THIS FUNCTION BODY
    int status = SUCCESSFUL;
    int i = 0;
    while (p_cmd->argv[i] != NULL){
        if (p_cmd->argv[i][strlen(p_cmd->argv[i])-1] == '\n'){
            p_cmd->argv[i][strlen(p_cmd->argv[i])-1] = '\0';
        }
        i++;
    }
    if (p_cmd->path[strlen(p_cmd->path)-1] == '\n'){
        p_cmd->path[strlen(p_cmd->path)-1] = '\0';
    }
    //write(1,p_cmd->path,strlen(p_cmd->path));

    //int child_process_status;
    pid_t child_pid = fork();
    if (p_cmd->argc > 0) {
        if (child_pid < 0) {
            perror("fork error");
        } else if (child_pid == 0) {
            if (is_builtin(p_cmd)) {
                do_builtin(p_cmd);
                return SUCCESSFUL;
            }

            execvp(p_cmd->path, p_cmd->argv);
            exit(1);
        }
        wait(NULL);
        return status;
    }
    else {
       perror("command does not exist");
       status = ERROR;
       return status;
   }

} // end execute function


int is_builtin( command_t* p_cmd ) {

    int cnt = 0;

    while ( valid_builtin_commands[cnt] != NULL ) {

        if ( equals( p_cmd->path, valid_builtin_commands[cnt] ) ) {

            return TRUE;

        }

        cnt++;

    }

    return FALSE;

} // end is_builtin function


int do_builtin( command_t* p_cmd ) {

    int i = 0;//THE FOLLOWING MODIFICATIONS ARE TO PASS AUTOGRADER TESTS SINCE
              //THE LAST CHAR OF ARGUMENT IS ALWAYS NEW LINE
    while (p_cmd->argv[i] != NULL){
        if (p_cmd->argv[i][strlen(p_cmd->argv[i])-1] == '\n'){
            p_cmd->argv[i][strlen(p_cmd->argv[i])-1] = '\0';
        }
        i++;
    }

    // only builtin command is cd

    if ( DEBUG ) printf("[builtin] (%s,%d)\n", p_cmd->path, p_cmd->argc);

    struct stat buff;
    int status = ERROR;
    //write(1,p_cmd->argv[1],strlen(p_cmd->argv[1]));

    if ( p_cmd->argc == 1 ) {

        // -----------------------
        // cd with no arg
        // -----------------------
        // change working directory to that
        // specified in HOME environmental
        // variable

        status = chdir( getenv("HOME") );

    } else if ( ( stat( p_cmd->argv[1], &buff ) == 0 && ( S_IFDIR & buff.st_mode ) ) ) {


        // -----------------------
        // cd with one arg
        // -----------------------
        // only perform this operation if the requested
        // folder exists

        status = chdir( p_cmd->argv[1] );

    }

    return status;

} // end do_builtin function



void cleanup( command_t* p_cmd ) {

    int i=0;

    while ( p_cmd->argv[i] != NULL ) {
        free( p_cmd->argv[i] );
        i++;
    }

    free( p_cmd->argv );
    free( p_cmd->path );

} // end cleanup function


int equals( char* str1, const char* str2 ) {
    int len[] = {0,0};

    char* b_str1 = str1;
    const char* b_str2 = str2;

    while( (*str1) != '\0' ) {
        len[0]++;
        str1++;
    }

    while( (*str2) != '\0' ) {
        len[1]++;
        str2++;
    }

    if ( len[0] != len[1] ) {

        return FALSE;

    } else {

        while ( (*b_str1) != '\0' ) {

            if ( tolower( (*b_str1)) != tolower((*b_str2)) ) {

                return FALSE;

            }

            b_str1++;
            b_str2++;

        }

    }
    //write(1,"return true\n",strlen("return true\n"));
    return TRUE;


} // end compare function definition
