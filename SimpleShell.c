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
    // Display the only history that was written in the shell before running this program and not the whole history from the text file
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

void execute_command(char **args) {
    pid_t pid;
    int status;
    struct timeval start, end;

    if (strcmp(args[0], "cd") == 0) {
        if (chdir(args[1]) < 0) {
            perror("chdir");
        }
        // else add the cd command in history
        gettimeofday(&start, NULL);
        gettimeofday(&end, NULL);
        long duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        add_to_history(args, pid, start, duration);
        return;
    }

    if (strcmp(args[0], "history") == 0){
        if (strcmp(args[1], "|") == 0){
            // now give the other command history.txt file to execute
            char *args2[MAX_LINE];
            int i;
            for (i = 2; args[i] != NULL; i++) {
                args2[i-2] = args[i];
            }
            // at the end give history.txt file as well in args2
            args2[i-2] = HISTORY_FILE;
            args2[i-1] = NULL;
            gettimeofday(&start, NULL);
            pid = fork();
            if (pid == 0) {
                execvp(args2[0], args2);
                perror("Execution Error1");
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("fork process failed");
            } else {
                waitpid(pid, &status, 0);
                gettimeofday(&end, NULL);
                long duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
                add_to_history(args, pid, start, duration);
            }
        }
        return;
    }
            

    // copy the args array to another ogcmd array
    char *ogcmd[MAX_LINE];
    int i;
    for (i = 0; args[i] != NULL; i++) {
        ogcmd[i] = args[i];
    }
    ogcmd[i] = NULL;
    // check for pipeline in command
    int num_pipes = 0;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            num_pipes++;
        }
    }
    if (num_pipes > 0) {
        pid_t id_s[num_pipes + 1];
        int arr[num_pipes * 2];
        int starter = 0, ender = 0;

        for (int j = 0; j < num_pipes; j++) {
            if (pipe(arr + j * 2) == -1) {
                perror("Failed to create pipe");
                exit(EXIT_FAILURE);
            }
        }
        for (int j = 0; j <= num_pipes; j++) {
            int ind1 = 0;
            while (args[ender] != NULL && strcmp(args[ender], "|") != 0) {
                ind1++;
                ender++;
            }
            args[ender] = NULL;  
            gettimeofday(&start, NULL);
            pid_t id_new = fork();
            if (id_new == 0) {
                if (j < num_pipes) {
                    dup2(arr[j * 2 + 1], STDOUT_FILENO);
                }
                if (j > 0) {
                    dup2(arr[(j - 1) * 2], STDIN_FILENO);
                }
                for (int k = 0; k < num_pipes * 2; k++) {
                    close(arr[k]);
                }
                execvp(args[starter], args + starter);
                perror("Execution Error");
                exit(EXIT_FAILURE);
            } else if (id_new < 0) {
                perror("fork process failed");
            } else {
                id_s[j] = id_new;
                starter = ++ender;
            }
        }

        for (int j = 0; j < num_pipes * 2; j++) {
            close(arr[j]);
        }
        for (int j = 0; j <= num_pipes; j++) {
            waitpid(id_s[j], &status, 0);
        }
        gettimeofday(&end, NULL);
        long duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        add_to_history(ogcmd, id_s[num_pipes], start, duration);
        return;
    }

    // execute single command
    gettimeofday(&start, NULL);
    pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("Execution Error");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork process failed");
    } else {
        waitpid(pid, &status, 0);
        gettimeofday(&end, NULL);
        long duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        add_to_history(ogcmd, pid, start, duration);
    }
}

void parse_command(char *cmd, char **args) {
    char *token;
    int i = 0;

    token = strtok(cmd, " \t\n");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

int main(int argc, char *argv[]) {
    char cmd[MAX_LINE];
    char *args[MAX_LINE/2 + 1];
    int should_run = 1;

    if (argc > 1) {
        // Read commands from the script file
        FILE *script_file = fopen(argv[1], "r");
        if (script_file == NULL) {
            perror("Failed to open script file");
            return 1;
        }

        char line[MAX_LINE];
        while (fgets(line, sizeof(line), script_file) != NULL) {
            // Parse the command and execute it
            parse_command(line, args);
            execute_command(args);
        }

        fclose(script_file);
        return 0;
    }

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
