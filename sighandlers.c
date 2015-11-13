/* mshell - a job manager */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

#include "jobs.h"
#include "common.h"
#include "sighandlers.h"

/*
 * wrapper for the sigaction function
 */
int sigaction_wrapper(int signum, handler_t * handler) {
    struct sigaction sa;
    
    if (verbose)
        printf("sigaction_wrapper: entering\n");
    
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(signum, &sa, NULL);

    if (verbose)
        printf("sigaction_wrapper: exiting\n");
    return 1;
}

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void sigchld_handler(int sig) {
    int pid, status;

    if (verbose)
        printf("sigchld_handler: entering\n");

    while((pid = waitpid(-1, &status, WUNTRACED|WNOHANG)) > 0){
        if (verbose){
            if(WIFEXITED(status)){
                printf("Fils terminé. PID : %d \tStatus : %d\n", pid, status);
            }
            if(WIFSIGNALED(status)){
                printf("Fils terminé. PID : %d \tSignal reçu : %d\n", pid, WTERMSIG(status));
            }
            if(WIFSTOPPED(status)){
                printf("Fils stoppé. PID : %d \tStatus : %d\n", pid, status);
            }
        }
        if(WIFEXITED(status) || WIFSIGNALED(status)){
            jobs_deletejob(pid);
        } else if(WIFSTOPPED(status)){
            struct job_t *job;
            job = jobs_getjobpid(pid);
            if(job != NULL){
                job->jb_state = ST;
            }
        }
    }

    if (verbose)
        printf("sigchld_handler: exiting\n");

    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {
    int pid;

    if (verbose)
        printf("sigint_handler: entering\n");

    pid = jobs_fgpid();
    kill(-pid, SIGINT);

    if (verbose)
        printf("sigint_handler: exiting\n");

    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {
    int pid;

    if (verbose)
        printf("sigtstp_handler: entering\n");

    pid = jobs_fgpid();
    kill(-pid, SIGTSTP);

    if (verbose)
        printf("sigtstp_handler: exiting\n");

    return;
}
