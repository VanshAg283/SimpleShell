#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_LINE 100 /* The maximum length command */
#define HISTORY_FILE "history.txt"

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

void handle_sigint(int sig) {
    printf("\nCtrl+C pressed. Displaying history:\n");
    display_history();
    exit(0);
}

void save_history_to_file() {
    FILE *file = fopen(HISTORY_FILE, "w");
    if (file == NULL) {
        perror("Failed to open history file");
        return;
    }

    for (int i = 0; i < history_count; i++) {
        for (int j = 0; j < MAX_LINE; j++) {
            fprintf(file, "%s ", history[i].cmd[j]);
        }
        fprintf(file, "\n");
        fprintf(file, "PID: %d\n", history[i].pid);
        fprintf(file, "Start Time: %ld seconds, %d microseconds\n", history[i].start.tv_sec, history[i].start.tv_usec);
        fprintf(file, "Duration: %ld milliseconds\n", history[i].duration);
        fprintf(file, "\n");
    }

    fclose(file);
}

void load_history_from_file() {
    FILE *file = fopen(HISTORY_FILE, "r");
    if (file == NULL) {
        perror("Failed to open history file");
        return;
    }

    char line[MAX_LINE];
    int cmd_index = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strcmp(line,"\n") == 0){
            continue;

        } else {
            char first_word[MAX_LINE];
            sscanf(line, "%s", first_word);

            if (strcmp(first_word, "PID:") == 0) {
                cmd_index = 0;
                history[history_count].pid = 0;
                sscanf(line, "PID: %d", &history[history_count].pid);
            } else if (strcmp(first_word, "Start") == 0) {
                sscanf(line, "Start Time: %ld seconds, %d microseconds", &history[history_count].start.tv_sec, &history[history_count].start.tv_usec);
            } else if (strcmp(first_word, "Duration:") == 0) {
                sscanf(line, "Duration: %ld milliseconds", &history[history_count].duration);
                history_count++;
            } else{
                char *token;
                token = strtok(line, " \t\n");
                while (token != NULL) {
                    strcpy(history[history_count].cmd[cmd_index++], token);
                    token = strtok(NULL, " \t\n");
                }
            }
        }
    }

    fclose(file);
}

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
    
    // Register signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    // Load history from file
    load_history_from_file();

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
