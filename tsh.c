/* 
 * tsh - A tiny shell program with job control
 * 
 * Name: Jacob West
 * NetID: wjacoba
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
int parseargs(char **argv, int *cmds, int *stdin_redir, int *stdout_redir);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}

/* 
 * parseargs - Parse the arguments to identify pipelined commands
 * 
 * Walk through each of the arguments to find each pipelined command.  If the
 * argument was | (pipe), then the next argument starts the new command on the
 * pipeline.  If the argument was < or >, then the next argument is the file
 * from/to which stdin or stdout should be redirected, respectively.  After it
 * runs, the arrays for cmds, stdin_redir, and stdout_redir all have the same
 * number of items---which is the number of commands in the pipeline.  The cmds
 * array is populated with the indexes of argv corresponding to the start of
 * each command sequence in the pipeline.  For each slot in cmds, there is a
 * corresponding slot in stdin_redir and stdout_redir.  If the slot has a -1,
 * then there is no redirection; if it is >= 0, then the value corresponds to
 * the index in argv that holds the filename associated with the redirection.
 * 
 */
int parseargs(char **argv, int *cmds, int *stdin_redir, int *stdout_redir) 
{
    int argindex = 0;    /* the index of the current argument in the current cmd */
    int cmdindex = 0;    /* the index of the current cmd */
    if (!argv[argindex]) {
        return 0;
    }

    cmds[cmdindex] = argindex;
    stdin_redir[cmdindex] = -1;
    stdout_redir[cmdindex] = -1;
    argindex++;
    while (argv[argindex]) {
        if (strcmp(argv[argindex], "<") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
	    }
            stdin_redir[cmdindex] = argindex;
	} else if (strcmp(argv[argindex], ">") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
	    }
            stdout_redir[cmdindex] = argindex;
	} else if (strcmp(argv[argindex], "|") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
	    }
            cmdindex++;
            cmds[cmdindex] = argindex;
            stdin_redir[cmdindex] = -1;
            stdout_redir[cmdindex] = -1;
	}
        argindex++;
    }

    return cmdindex + 1;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

pid_t protectedFork() {
    pid_t pid;
    if((pid = fork()) < 0) {
        unix_error("Error forking.\n");
    }
    return pid;
}

void protectedSetpgid(pid_t pid, pid_t processGroupId) {
    if(setpgid(pid, processGroupId) < 0) {
        unix_error("Error calling setpgid().");
    }
}

void protectedSigprocmask(int operation, sigset_t *mask, sigset_t *old_mask) {
    if(sigprocmask(operation, mask, old_mask) < 0) {
        app_error("Error calling sigprocmask().");
    }
}

void protectedSigaddset(sigset_t *mask, int signal) {
    if(sigaddset(mask, signal) < 0) {
        app_error("Error calling sigemptyset().");
    }
}

void protectedSigemptyset(sigset_t *mask) {
    if(sigemptyset(mask) < 0) {
        app_error("Error calling sigemptyset().");
    }
}

void protectedKill(pid_t pid, int sig) {
    if(kill(pid, sig) < 0) {
        unix_error("Error calling kill.");
    }
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) {
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
// *****************************************************
// *********************TODO: 50 lines*************************
// *****************************************************
void do_bgfg(char **argv) {
    
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
// *****************************************************
// *********************TODO: 20 lines*************************
// *****************************************************
void waitfg(pid_t pid) {
    // – In waitfg, use a busy loop around the sleep function.
    
    while(1) {
        // fgpid(jobs) returns the pid of the current foreground job, 
        // or 0 if there isn't a foreground job
        if(pid != fgpid(jobs)) {
            break; // We are done waiting.
        } else {
            sleep(1);
        }
    }
}

/********************************************************************
 * Signal handlers. I implement these
 ********************************************************************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 * SIGCHLD sent to kernel when child process terminates.
 */
// *****************************************************
// *********************TODO: 80 lines*************************
// *****************************************************
void sigchld_handler(int sig) {

    // Reap the child process using waitpid

    pid_t pid;
    int status;

    // Since there may be more than one child process waiting to be reaped when sigchld_handler()
    // is called, you need to call waitpid() in a loop until it returns something less than 0. 
    // By reaping and deleting jobs in a loop, you ensure that all zombie processes are reaped
    // and no jobs are left unaccounted for.
    while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) { // Returns a pid

        int jobId = pid2jid(pid); // Needed for future functionality?

        //********waitpid() explanation below**************//
        // pid - 1 means you want to wait for any child (effectively making waitpid() behave like wait())
        // WNOHANG means waitpid will return immediately instead of blocking
        // and WUNTRACED means stopped processes will be reaped
        // Waitpid returns 0 if no children have terminated, or with the PID of one of the terminated children.

        // WIFEXITED returns true if the child terminated normally
        if(WIFEXITED(status)) {
            deletejob(jobs, pid);   
            if (verbose) printf("sigchld_handler: jobId %d, pid %d, deleted.\n", jobId, pid);
            if (verbose) printf("sigchld_handler: jobId %d, pid %d, terminated normally. Exit status: %d.\n", jobId, pid, WEXITSTATUS(status));
        }

        // WIFSIGNALED returns true if the child process was terminated by a signal (like SIGINT if they 
        // hit us up with ctrl-c.
        if(WIFSIGNALED(status)) {
            deletejob(jobs, pid);
            if (verbose) printf("sigchld_handler: jobId %d, pid %d, deleted.\n", jobId, pid);
            printf("Job [%d] (%d) terminated by signal %d\n", jobId, (int) pid, WTERMSIG(status));
        }

        // WIFSTOPPPED returns true if the child process was stopped by delivery of a signal. (like if
        // a child were terminated)
        if(WIFSTOPPED(status)) {
            // change job's tracked state from FG to ST
            getjobpid(jobs, pid)->state = ST;
            printf("Job [%d] (%d) stopped by signal %d\n", jobId, (int) pid, WSTOPSIG(status));
        }
        
    }

    // (PDF)
    // get the job associated with the terminated process' pid
    // delete job from jobs table
    // Back to the input loop (aka return)

    // (PDF) -- if triggered by the SIGSTP handler
    // get the job associated with the terminated process' pid
    // print result to console
    // Back to the input loop (aka return)

    // (SLIDES)
    // reap the child process using waitpid();

}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
// *****************************************************
// *********************TODO: 15 lines*************************
// *****************************************************
void sigint_handler(int sig) {
    // get the foreground job
    pid_t foregroundPid = fgpid(jobs);

    if(foregroundPid != 0) { // Don't do anything if we are inside of the child?
        // terminate foreground job (and all processes in the same process group)
        protectedKill(-foregroundPid, sig);
    }

    
    // (PDF)
    // get foreground job
    // terminate foreground job (and all processes in the same process group)
    // Which will trigger the SIGSTP handler
    // Back to the input loop (aka return)
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 * 
 */
// *****************************************************
// *********************TODO: 15 lines*************************
// *****************************************************
void sigtstp_handler(int sig) {

    // get the foreground job
    pid_t foregroundPid = fgpid(jobs);

    if(foregroundPid != 0) { // Don't do anything if we are inside of the child?
        // terminate foreground job (and all processes in the same process group)
        protectedKill(-foregroundPid, sig);
    }

    // (PDF) -- If triggered by SIGINT
    // get the job associated with the signalled process pid
    // delete foreground job
    // print resul to the console.
    // Back to the input loop (aka return)

}

/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
// *****************************************************
// *********************TODO: 70 lines*************************
// *****************************************************
void eval(char *cmdline) 
{
    //################### Variables ######################//
    char *argv[MAXLINE];    // Using MAXLINE instead of MAXARGS because this will contain 
                            // both arguments and commands.
    char arguments[100];    // Will contain the arguments from the commandline.
    strcpy(arguments, cmdline);
    
    int isBackgroundJob; // Will be 1 if user has requesteed a background job
                          // Will be 0 if user has requested a foreground job
    isBackgroundJob = parseline(arguments, argv);  // Will be 1 if user has requested a BG job
                                                   // Will be 0 if user has requested a FG job

    if(!builtin_cmd(argv)) {  

        pid_t pid = 0;

        // The parent must use sigprocmask to block SIGCHLD signals before it forks the child
        sigset_t mask;
        protectedSigemptyset(&mask);          // init vector to zeros
        protectedSigaddset(&mask, SIGCHLD); // set the SIGCHLD bit in the vector
        protectedSigprocmask(SIG_BLOCK, &mask, NULL); // Block the sigchild bit in the vector

        // Child
        if((pid = protectedFork()) == 0) {
            //before the execve, the child process should call
            // setpgid(0, 0), which puts the child in a new process group whose group ID is identical to the
            // child’s PID. This ensures that there will be only one process, your shell, in the foreground process
            // group.
            protectedSetpgid(0,0);
            
            // Unblock SIGCHLD signals, again using sigprocmask after it adds the child
            protectedSigprocmask(SIG_UNBLOCK, &mask, NULL); // Unblock the sigchild bit in the vector

            // Attempt to execute the program
            if (execve(argv[0], argv, environ) < 0) {
                    printf("%s: Command not found\n", argv[0]);
                    exit(0); // Exit the child process
            }

        }

        // Parent
        if(isBackgroundJob) { // Background job
            // Add job to list
            addjob(jobs, pid, BG, cmdline);
            protectedSigprocmask(SIG_UNBLOCK, &mask, NULL);
            printf("[%d] (%d) %s\n", pid2jid(pid), (int)pid, cmdline);               
            // unblock the blocked SIGCHLD signals using sigprocmask after 
            // it adds the child to the job list by calling addjob.
        } else { // Foreground
            // Add job to list
            addjob(jobs, pid, FG, cmdline);
            protectedSigprocmask(SIG_UNBLOCK, &mask, NULL);
            waitfg(pid); // Wait on the foreground process
        }        
    }      



    // In eval, the parent must use sigprocmask to block SIGCHLD signals before it forks the child,
    // and then unblock these signals, again using sigprocmask after it adds the child to the job list by
    // calling addjob. Since children inherit the blocked vectors of their parents, the child must be sure
    // to then unblock SIGCHLD signals before it execs the new program.
    // The parent needs to block the SIGCHLD signals in this way in order to avoid the race condition where
    // the child is reaped by sigchld handler (and thus removed from the job list) before the parent
    // calls addjob.

    // After the fork, but before the execve, the child process should call
    // setpgid(0, 0), which puts the child in a new process group whose group ID is identical to the
    // child’s PID. This ensures that there will be only one process, your shell, in the foreground process
    // group. When you type ctrl-c, the shell should catch the resulting SIGINT and then forward it
    // to the appropriate foreground job (or more precisely, the process group that contains the foreground
    // job).
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
// *****************************************************
// *********************TODO: 50 lines*************************
// *****************************************************
int builtin_cmd(char **argv) 
{
    if(!strcmp(argv[0], "quit")) { // If firstCommand == "quit"
        exit(0);
    }
    if(!strcmp(argv[0], "fg")) { // If firstCommand == "fg"
        do_bgfg(argv);
        return 1;
    }
    if(!strcmp(argv[0], "bg")) { // If firstCommand == "bg"
        do_bgfg(argv);
        return 1;
    }
    if(!strcmp(argv[0], "jobs")) { // If firstCommand == "jobs"
        listjobs(jobs);
        return 1;
    }
    return 0;     /* not a builtin command */
}

/************************************************************************
 * End signal handlers. I implement these
 ************************************************************************/

/***********************************************
 * Helper routines that manipulate the job list. These are provided to me.
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/