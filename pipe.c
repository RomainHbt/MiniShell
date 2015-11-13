/* mshell - a job manager */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "pipe.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
	int fds[2];
	int pid;
	int erreur;

    if (verbose)
        printf("do_pipe: entering\n");
    
    /* nbcmd = 2 */

    if(nbcmd == 2){

    	erreur = pipe(fds);
    	if(erreur != 0){
    		perror("pipe");
    		exit(EXIT_FAILURE);
    	}

    	switch((pid = fork())){
    		case -1 :
    			perror("fork");
    			exit(EXIT_FAILURE);
    		case 0:
    			/* FILS */
    			if (verbose) printf("Commande 1\n");
	    		erreur = dup2(fds[1], STDOUT_FILENO);
	    		if(erreur == -1){
	    			perror("dup2");
	    			exit(EXIT_FAILURE);
	    		}
	    		close(fds[0]);
	    		close(fds[1]);
	    		execvp(cmds[0][0], cmds[0]);
	    		exit(EXIT_FAILURE);
    	}

    	switch(fork()){
    		case -1 :
    			perror("fork");
    			exit(EXIT_FAILURE);
    		case 0:
    			/* FILS */
    			if (verbose) printf("Commande 2\n");
	    		erreur = dup2(fds[0], STDIN_FILENO);
	    		if(erreur == -1){
	    			perror("dup2");
	    			exit(EXIT_FAILURE);
	    		}
	    		close(fds[0]);
	    		close(fds[1]);
	    		execvp(cmds[1][0], cmds[1]);
	    		exit(EXIT_FAILURE);
    	}

    	close(fds[0]);
	    close(fds[1]);
	    wait(NULL);
	    wait(NULL);

    } else {
    	printf("pipe : To be implemented\n");
    }


    if (verbose)
        printf("do_pipe: exiting\n");

    return;
}
