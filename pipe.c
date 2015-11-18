/* mshell - a job manager */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "pipe.h"
#include "jobs.h"
#include "cmd.h"

void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
	int fds[2][2];
	int pid;
	int erreur;

    if (verbose)
        printf("do_pipe: entering\n");
    
    if(nbcmd == 2){

    	erreur = pipe(fds[0]);
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
    			setpgid(0,0);
	    		erreur = dup2(fds[0][1], STDOUT_FILENO);
	    		if(erreur == -1){
	    			perror("dup2");
	    			exit(EXIT_FAILURE);
	    		}
	    		close(fds[0][0]);
	    		close(fds[0][1]);
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
    			setpgid(0, pid);
	    		erreur = dup2(fds[0][0], STDIN_FILENO);
	    		if(erreur == -1){
	    			perror("dup2");
	    			exit(EXIT_FAILURE);
	    		}
	    		close(fds[0][0]);
	    		close(fds[0][1]);
	    		execvp(cmds[1][0], cmds[1]);
	    		exit(EXIT_FAILURE);
    	}

    	close(fds[0][0]);
	    close(fds[0][1]);

    } else if(nbcmd == 3) {

        erreur = pipe(fds[0]);
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
                setpgid(0,0);
                erreur = dup2(fds[0][1], STDOUT_FILENO);
                if(erreur == -1){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fds[0][0]);
                close(fds[0][1]);
                execvp(cmds[0][0], cmds[0]);
                exit(EXIT_FAILURE);
        }

        erreur = pipe(fds[1]);
        if(erreur != 0){
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        switch(fork()){
            case -1 :
                perror("fork");
                exit(EXIT_FAILURE);
            case 0:
                /* FILS */
                if (verbose) printf("Commande 2\n");
                setpgid(0, pid);
                erreur = dup2(fds[0][0], STDIN_FILENO);
                if(erreur == -1){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                erreur = dup2(fds[1][1], STDOUT_FILENO);
                if(erreur == -1){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fds[0][0]);
                close(fds[0][1]);
                close(fds[1][0]);
                close(fds[1][1]);
                execvp(cmds[1][0], cmds[1]);
                exit(EXIT_FAILURE);
        }

        close(fds[0][0]);
        close(fds[0][1]);

        switch(fork()){
            case -1 :
                perror("fork");
                exit(EXIT_FAILURE);
            case 0:
                /* FILS */
                if (verbose) printf("Commande 3\n");
                setpgid(0, pid);
                erreur = dup2(fds[1][0], STDIN_FILENO);
                if(erreur == -1){
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
                close(fds[1][0]);
                close(fds[1][1]);
                execvp(cmds[2][0], cmds[2]);
                exit(EXIT_FAILURE);
        }

        close(fds[1][0]);
        close(fds[1][1]);

    } else {
        for (int i = 0; i < nbcmd; ++i) {
            if(i == 0){
                /* Première commande */
                switch((pid = fork())){
                    case -1 :
                        perror("fork");
                        exit(EXIT_FAILURE);
                    case 0:
                        /* FILS */
                        if (verbose) printf("Commande %d\n", i);
                        setpgid(0,0);
                        erreur = dup2(fds[0][1], STDOUT_FILENO);
                        if(erreur == -1){
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                        close(fds[0][0]);
                        close(fds[0][1]);
                        execvp(cmds[0][0], cmds[0]);
                        exit(EXIT_FAILURE);
                }
            } else if (i == nbcmd-1){
                /* Dernière commande */
            } else {
                /* Autre commande */
            }
        }
    }

    if(bg){
        /* Arrière-plan */
        jobs_addjob(pid, BG, cmds[0][0]);
    } else {
        /* Avant-plan */
        jobs_addjob(pid, FG, cmds[0][0]);
        waitfg(pid);
    }

    if (verbose)
        printf("do_pipe: exiting\n");

    return;
}
