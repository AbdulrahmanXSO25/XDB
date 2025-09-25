/*-------------------------------------------------------------------------
 *
 * main.c
 *	  Main entry point for XDB executable.
 *-------------------------------------------------------------------------
 */
#include "xdb.h"

#define MAX_INPUT_LEN (1 << 9)

void trim(char *s) {
    char *p = s;
    while (*p) {
        if (*p == '\n' || *p == '\r') *p = 0;
        p++;
    }
}

#define NOT_IMPLEMENTED_MSG(cmd) \
    printf("%s not implemented yet. Command was: %s\n", cmd, input);

void cmd_new_rel(char *input) {
    NOT_IMPLEMENTED_MSG("NEW REL");
}

void cmd_drop_rel(char *input) {
    NOT_IMPLEMENTED_MSG("DROP REL");
}

void cmd_list_rels(char *input) {
    NOT_IMPLEMENTED_MSG("LIST RELS");
}

void cmd_insert(char *input) {
    NOT_IMPLEMENTED_MSG("INSERT");
}

void cmd_scan(char *input) {
    NOT_IMPLEMENTED_MSG("SCAN");
}

void cmd_delete(char *input) {
    NOT_IMPLEMENTED_MSG("DELETE");
}

void cmd_update(char *input) {
    NOT_IMPLEMENTED_MSG("UPDATE");
}

void cmd_help() {
    printf("Commands:\n");
    printf("  new rel name(type, type, ...)\n");
    printf("  drop rel name\n");
    printf("  list rels\n");
    printf("  insert relname values(v1, v2,...)\n");
    printf("  scan relname\n");
    printf("  delete ...        (placeholder)\n");
    printf("  update ...        (placeholder)\n");
    printf("  help\n");
    printf("  exit\n");
}

void cmd_clear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void cmd_exit() {
    printf("Exiting XDB.\n");
    exit(EXIT_SUCCESS);
}

void process_command(char *input) {
    if (strncmp(input, "new rel", 7) == 0) {
        cmd_new_rel(input);
    } else if (strncmp(input, "drop rel", 8) == 0) {
        cmd_drop_rel(input);
    } else if (strncmp(input, "list rels", 9) == 0) {
        cmd_list_rels(input);
    } else if (strncmp(input, "insert", 6) == 0) {
        cmd_insert(input);
    } else if (strncmp(input, "scan", 4) == 0) {
        cmd_scan(input);
    } else if (strncmp(input, "delete", 6) == 0) {
        cmd_delete(input);
    } else if (strncmp(input, "update", 6) == 0) {
        cmd_update(input);
    } else if (strncmp(input, "help", 4) == 0) {
        cmd_help();
    } else if (strncmp(input, "clear", 5) == 0) {
        cmd_clear();
    } else if (strncmp(input, "exit", 4) == 0) {
        cmd_exit();
    } else {
        printf("Unrecognized command: %s\n", input);
    }
}

int main(int argc, char *argv[]) {
    
    char input[MAX_INPUT_LEN];

    printf("XDB Version: %s\n", XDB_VERSION);
    printf("Enter commands (type 'help' for commands):\n");
    while (1) {
        printf("xdb> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        trim(input);
        
        if (strlen(input) == 0) continue;

        process_command(input);
    }

    return EXIT_SUCCESS;
}