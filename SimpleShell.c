#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_LINE 100 /* The maximum length command */

struct Command{
    char cmd[MAX_LINE][MAX_LINE];
    pid_t pid;
    struct timeval start,end;
    long duration;
};

struct Command history[MAX_LINE];
int history_count = 0;

void display_history() {
    for (int i = 0; i < history_count; i++) {
        printf("Command: ");
        for (int j = 0; j < MAX_LINE; j++) {
            printf("%s ", history[i].cmd[j]);
        }
        printf("\n");
        printf("PID: %d\n", history[i].pid);
        printf("Start Time: %ld seconds, %d microseconds\n", history[i].start.tv_sec, history[i].start.tv_usec);
        printf("Duration: %ld milliseconds\n", history[i].duration);
        printf("\n");
    }
}

void add_to_history(char **cmd, pid_t pid, struct timeval start, long duration) {
    for (int i = 0; cmd[i] != NULL; i++) {
        strcpy(history[history_count].cmd[i], cmd[i]);
    }
    history[history_count].pid = pid;
    history[history_count].start = start;
    history[history_count].duration = duration;
    history_count++;
}
void handle_sigint(int sig) {printf("\nCtrl+C pressed. Displaying history:\n");// Display the only history that was written in the shell before running this program and not the whole history from the text filedisplay_history();exit(0);}

void parse_command(char *cmd, char **args) {
char *token;
int i = 0;
token = strtok(cmd, " \t\n");
while (token != NULL) {
    args[i++] = token;
    token = strtok(NULL, " \t\n");
}
args[i] = NULL;
}

int main(int argc, char *argv[]) {
    char cmd[MAX_LINE];
    char *args[MAX_LINE/2 + 1];
    int should_run = 1;
    / Register signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    
    do {
    printf("SimpleShell> ");
    fgets(cmd, MAX_LINE, stdin);
    cmd[strlen(cmd) - 1] = '\0'; // remove newline at the end

    if (strcmp(cmd, "") == 0) {
        continue; // Skip further execution if command is empty
    }

    if (strcmp(cmd, "exit") == 0) {
        should_run = 0;
        continue;
    }

    if (strcmp(cmd, "history") == 0) {
        for (int i = 0; i < history_count; i++) {
            printf("%d  ",i+1);
            for (int j = 0; j < MAX_LINE; j++) {
                printf("%s ", history[i].cmd[j]);
            }
            printf("\n");
        }
        continue;
    }

    parse_command(cmd, args);

    execute_command(args);
    // Save history to file
    save_history_to_file();
    } while (should_run);

    return 0;
}
