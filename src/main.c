/*-------------------------------------------------------------------------
 *
 * main.c
 *	  Main entry point for XDB executable.
 *-------------------------------------------------------------------------
 */
#include "xdb.h"

static const AttrDesc ATTR_TYPES[] = {
    {"INT", ATTR_TYPE_INT, sizeof(int32_t)},
    {"BIGINT", ATTR_TYPE_BIGINT, sizeof(int64_t)},
    {"FLOAT", ATTR_TYPE_FLOAT, sizeof(float)},
    {"DOUBLE", ATTR_TYPE_DOUBLE, sizeof(double)},
    {"CHAR", ATTR_TYPE_CHAR, sizeof(char)},
    {"BOOL", ATTR_TYPE_BOOL, sizeof(uint8_t)},
    {"TIMESTAMP", ATTR_TYPE_TIMESTAMP, sizeof(int64_t)},
    {"TEXT", ATTR_TYPE_TEXT, 256}
};

int get_attr_type(const char *type_name) {
    for (size_t i = 0; i < sizeof(ATTR_TYPES)/sizeof(AttrDesc); i++) {
        if (strcmp(ATTR_TYPES[i].name, type_name) == 0) {
            return ATTR_TYPES[i].type;
        }
    }
    return -1;
}

const char* get_attr_type_name(int type) {
    for (size_t i = 0; i < sizeof(ATTR_TYPES)/sizeof(AttrDesc); i++) {
        if (ATTR_TYPES[i].type == type) {
            return ATTR_TYPES[i].name;
        }
    }
    return "UNKNOWN";
}

static Relation rels[MAX_RELS] = {0};
static int nrels = 0;

void initialize_db() {
    nrels = 0;
    memset(rels, 0, sizeof(rels));
}

/* 0 on success, -1 on error */
int add_rel(const char *name, TupleDesc *tuple_desc) {
    if (nrels >= MAX_RELS) {
        return -1;
    }
    for (int i = 0; i < nrels; i++) {
        if (strcmp(rels[i].name, name) == 0) {
            return -1;
        }
    }
    Relation *rel = &rels[nrels++];
    strncpy(rel->name, name, MAX_REL_NAME_LEN);
    rel->tuple_desc = *tuple_desc;
    rel->ntuples = 0;
    return 0;
}

/* 0 on success, -1 on error */
int del_rel(const char *name) {
    for (int i = 0; i < nrels; i++) {
        if (strcmp(rels[i].name, name) == 0) {
            rels[i] = rels[--nrels];
            return 0;
        }
    }
    return -1;
}

const char* list_rels() {
    static char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    for (int i = 0; i < nrels; i++) {
        char line[256];
        snprintf(line, sizeof(line), "Relation: %s, Attributes: ", rels[i].name);
        strcat(buffer, line);
        for (int j = 0; j < rels[i].tuple_desc.nattrs; j++) {
            const AttrDesc *attr = &rels[i].tuple_desc.attrs[j];
            snprintf(line, sizeof(line), "%s(%s)", attr->name, get_attr_type_name(attr->type));
            strcat(buffer, line);
            if (j < rels[i].tuple_desc.nattrs - 1) {
                strcat(buffer, ", ");
            }
        }
        strcat(buffer, "\n");
    }
    return buffer;
}

/* 0 on success, -1 on error */
int scan_rel(const char *rel_name, TupleDesc *out_desc, Tuple *out_tuples, uint32_t *out_ntuples) {
    for (int i = 0; i < nrels; i++) {
        if (strcmp(rels[i].name, rel_name) == 0) {
            Relation *rel = &rels[i];
            if (rel->ntuples >= MAX_TUPLES) {
                return -1;
            }

            *out_desc = rel->tuple_desc;
            *out_ntuples = rel->ntuples;
            for (uint32_t j = 0; j < rel->ntuples; j++) {
                out_tuples[j] = rel->tuples[j];
            }
            return 0;
        }
    }
    return -1;
}

void print_tuples(const TupleDesc *desc, const Tuple *tuples, uint32_t ntuples) {
    /* Print header */
    for (int i = 0; i < desc->nattrs; i++) {
        printf("%-20s", desc->attrs[i].name);
    }
    printf("\n");
    for (int i = 0; i < desc->nattrs; i++) {
        printf("--------------------");
    }
    printf("\n");

    /* Print rows */
    for (uint32_t i = 0; i < ntuples; i++) {
        for (int j = 0; j < desc->nattrs; j++) {
            const AttrDesc *attr = &desc->attrs[j];
            const Value *val = &tuples[i].values[j];
            switch (attr->type) {
                case ATTR_TYPE_INT:
                    printf("%-20d", val->i32);
                    break;
                case ATTR_TYPE_BIGINT:
                    printf("%-20lld", (long long)val->i64);
                    break;
                case ATTR_TYPE_FLOAT:
                    printf("%-20f", val->f);
                    break;
                case ATTR_TYPE_DOUBLE:
                    printf("%-20lf", val->d);
                    break;
                case ATTR_TYPE_CHAR:
                    printf("%-20c", val->c);
                    break;
                case ATTR_TYPE_BOOL:
                    printf("%-20s", val->b ? "true" : "false");
                    break;
                case ATTR_TYPE_TIMESTAMP:
                    printf("%-20lld", (long long)val->ts);
                    break;
                case ATTR_TYPE_TEXT:
                    printf("%-20s", val->str);
                    break;
                default:
                    printf("%-20s", "UNKNOWN");
            }
        }
        printf("\n");
    }
}

/* 0 on success, -1 on error */
int insert_tuple(const char *rel_name, const void **values) {
    for (int i = 0; i < nrels; i++) {
        if (strcmp(rels[i].name, rel_name) == 0) {
            Relation *rel = &rels[i];
            if (rel->ntuples >= MAX_TUPLES) {
                return -1;
            }
            Tuple *tup = &rel->tuples[rel->ntuples];
            
            tup->values = malloc(rel->tuple_desc.nattrs * sizeof(Value));
            if (!tup->values) return -1;

            for (int j = 0; j < rel->tuple_desc.nattrs; j++) {
                const AttrDesc *attr = &rel->tuple_desc.attrs[j];
                const char *str_val = (const char*)values[j];
                
                switch (attr->type) {
                    case ATTR_TYPE_INT: {
                        int32_t val = atoi(str_val);
                        tup->values[j].i32 = val;
                        break;
                    }
                    case ATTR_TYPE_BIGINT: {
                        int64_t val = atoll(str_val);
                        tup->values[j].i64 = val;
                        break;
                    }
                    case ATTR_TYPE_FLOAT: {
                        float val = atof(str_val);
                        tup->values[j].f = val;
                        break;
                    }
                    case ATTR_TYPE_DOUBLE: {
                        double val = atof(str_val);
                        tup->values[j].d = val;
                        break;
                    }
                    case ATTR_TYPE_CHAR:
                        tup->values[j].c = str_val[0];
                        break;
                    case ATTR_TYPE_BOOL:
                        tup->values[j].b = (strcmp(str_val, "true") == 0 || strcmp(str_val, "1") == 0);
                        break;
                    case ATTR_TYPE_TIMESTAMP: {
                        int64_t val = atoll(str_val);
                        tup->values[j].ts = val;
                        break;
                    }
                    case ATTR_TYPE_TEXT: {
                        tup->values[j].str = malloc(attr->size);
                        if (!tup->values[j].str) {
                            for (int k = 0; k < j; k++) {
                                if (rel->tuple_desc.attrs[k].type == ATTR_TYPE_TEXT) {
                                    free(tup->values[k].str);
                                }
                            }
                            free(tup->values);
                            return -1;
                        }
                        strncpy(tup->values[j].str, str_val, attr->size - 1);
                        tup->values[j].str[attr->size - 1] = '\0';
                        break;
                    }
                    default:
                        free(tup->values);
                        return -1;
                }
            }
            rel->ntuples++;
            return 0;
        }
    }
    return -1;
}

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
    
    printf("Initializing XDB...\n");
    initialize_db();

    printf("Adding sample relation 'persons'...\n");
    TupleDesc td;
    td.nattrs = 3;

    strncpy(td.attrs[0].name, "id", MAX_ATTR_NAME_LEN);
    td.attrs[0].type = ATTR_TYPE_INT;
    td.attrs[0].size = sizeof(int32_t);

    strncpy(td.attrs[1].name, "name", MAX_ATTR_NAME_LEN);
    td.attrs[1].type = ATTR_TYPE_TEXT;
    td.attrs[1].size = 256;

    strncpy(td.attrs[2].name, "age", MAX_ATTR_NAME_LEN);
    td.attrs[2].type = ATTR_TYPE_INT;
    td.attrs[2].size = sizeof(int32_t);

    printf("Adding sample relation 'persons'...\n");

    if (add_rel("persons", &td) != 0) {
        fprintf(stderr, "Failed to add relation 'persons'\n");
        return EXIT_FAILURE;
    }

    printf("Inserting sample tuples into 'persons'...\n");

    const char *vals1[] = {"1", "Alice", "30"};
    const char *vals2[] = {"2", "Bob", "25"};
    if (insert_tuple("persons", (const void **)vals1) != 0 ||
        insert_tuple("persons", (const void **)vals2) != 0) {
        fprintf(stderr, "Failed to insert tuples into 'persons'\n");
        return EXIT_FAILURE;
    }

    printf("Current Relations:\n%s", list_rels());

    printf("Adding another sample relation 'books'...\n");
    TupleDesc td2;
    td2.nattrs = 3;

    strncpy(td2.attrs[0].name, "book_id", MAX_ATTR_NAME_LEN);
    td2.attrs[0].type = ATTR_TYPE_INT;
    td2.attrs[0].size = sizeof(int32_t);

    strncpy(td2.attrs[1].name, "title", MAX_ATTR_NAME_LEN);
    td2.attrs[1].type = ATTR_TYPE_TEXT;
    td2.attrs[1].size = 256;

    strncpy(td2.attrs[2].name, "year", MAX_ATTR_NAME_LEN);
    td2.attrs[2].type = ATTR_TYPE_INT;
    td2.attrs[2].size = sizeof(int32_t);

    if (add_rel("books", &td2) != 0) {
        fprintf(stderr, "Failed to add relation 'books'\n");
        return EXIT_FAILURE;
    }

    printf("Inserting sample tuples into 'books'...\n");

    const char *book_vals1[] = {"101", "C Programming Language", "1978"};
    const char *book_vals2[] = {"102", "Clean Code", "2008"};
    if (insert_tuple("books", (const void **)book_vals1) != 0 ||
        insert_tuple("books", (const void **)book_vals2) != 0) {
        fprintf(stderr, "Failed to insert tuples into 'books'\n");
        return EXIT_FAILURE;
    }

    printf("Current Relations:\n%s", list_rels());

    printf("Deleting relation 'persons'...\n");
    if (del_rel("persons") != 0) {
        fprintf(stderr, "Failed to delete relation 'persons'\n");
        return EXIT_FAILURE;
    }

    printf("Current Relations after deletion:\n%s", list_rels());

    TupleDesc out_desc;
    Tuple out_tuples[MAX_TUPLES];
    uint32_t out_ntuples = 0;
    if (scan_rel("books", &out_desc, out_tuples, &out_ntuples) != 0) {
        fprintf(stderr, "Failed to scan relation 'books'\n");
        return EXIT_FAILURE;
    }

    printf("Scanned Tuples from 'books':\n");
    print_tuples(&out_desc, out_tuples, out_ntuples);

    // char input[MAX_INPUT_LEN];

    // printf("XDB Version: %s\n", XDB_VERSION);
    // printf("Enter commands (type 'help' for commands):\n");
    // while (1) {
    //     printf("xdb> ");
    //     if (!fgets(input, sizeof(input), stdin)) break;
    //     trim(input);
        
    //     if (strlen(input) == 0) continue;

    //     process_command(input);
    // }

    return EXIT_SUCCESS;
}