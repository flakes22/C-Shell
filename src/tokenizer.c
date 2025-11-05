#include"header.h"


//int check_pos=0;
int parse_shell_cmd();
int parse_output();
int valid_name(char *input);
int parse_name();
int parse_input();
int parse_atomic();
int parse_cmd_grp();
// Tokenizer------------------------------
int tokenizer(char *input)
{
    int token_count = 0;
    int length = strlen(input);

    for (int i = 0; i < length; i++)
    {
        while (input[i] == ' ')
            i++; 

        if (input[i] == '>' && (i + 1) < length && input[i + 1] == '>') // bound check
        {
            tokens[token_count++] = strdup(">>");
            i++;
        }
        else if (input[i] == '|' || input[i] == '&' || input[i] == ';' || input[i] == '<' || input[i] == '>')
        {
            char temp[2] = {input[i], '\0'};
            tokens[token_count++] = strdup(temp);
        }
        else
        {
            char buf[256];
            int j = 0;
            while (i < length && input[i] != ' ' &&
                   !(input[i] == '|' || input[i] == '&' || input[i] == ';' || input[i] == '<' || input[i] == '>' || input[i] == '?'))
            {
                buf[j++] = input[i++];
            }
            buf[j] = '\0';
            if (j > 0)
            {
                tokens[token_count++] = strdup(buf);
            }
            if (i < length && (input[i] == '?'))
            {
                char invalid[2] = {input[i], '\0'};
                tokens[token_count++] = strdup(invalid);
            }
        }
    }
    // printf("zero %d ",token_count);
    tok_count = token_count;
    tokens[token_count] = NULL;   
    pos = 0;                      
    check_pos = pos;            

    if (parse_shell_cmd()) {
        printf("Invalid Syntax!\n");
        return 1;
       // exit(1); // Exit immediately on error
    }
    return 0;
}

// strdup() is a POSIX library function that makes a heap-allocated copy of a string.

int valid_name(char *input) {
    if (!input) return 0;
    const char *pattern = "^[^|&><;]+$";
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED)) return 0;
    int result = regexec(&regex, input, 0, NULL, 0) == 0;
    regfree(&regex);
    return result;
}

int parse_name() {
    if (check_pos >= tok_count || !tokens[check_pos]) return 1; // no tokens
    if (valid_name(tokens[check_pos])) {
        (check_pos)++;
        return 0;  // success
    }
    return 1; // not a valid name
}

int parse_output() {
    if (check_pos >= tok_count || !tokens[check_pos]) return 1;
    // printf("one %d ",*check_pos);
    // printf("%s",tokens[*check_pos]);
    if (strcmp(tokens[check_pos], ">") == 0 || strcmp(tokens[check_pos], ">>") == 0) {
        (check_pos)++;
        fflush(stdout);
        // printf("hjf");
        
        if (check_pos >= tok_count || tokens[check_pos]==NULL || parse_name()) {
        //     printf("two %d ",*check_pos);
        // printf("%s",tokens[*check_pos]);
        
          //  printf("Invalid Syntax!\n");
            return 1;
        }
        return 0;
    }
    return 1;
}

int parse_input() {
    if (check_pos >= tok_count || !tokens[check_pos]) return 1;

    if (strcmp(tokens[check_pos], "<") == 0) {
        (check_pos)++;
        if (check_pos >= tok_count || !tokens[check_pos] || parse_name()) {
            //printf("Invalid Syntax!\n!");
            return 1;
        }
        return 0;
    }
    return 1;
}

int parse_atomic() {
    if (parse_name()) {
       // printf("Invalid Syntax!\n");
        return 1;
    }

    while (check_pos < tok_count && tokens[check_pos]) {
        if (strcmp(tokens[check_pos], "<") == 0) {
            if (parse_input()) return 1;
        } else if (strcmp(tokens[check_pos], ">") == 0 || strcmp(tokens[check_pos], ">>") == 0) {
            if (parse_output()) return 1;
        } 
        else if (valid_name(tokens[check_pos])) {
            if (parse_name()) return 1;
        } 
        else {
           return 0;
        }
    }
    return 0;
}

int parse_cmd_grp() {
    if (parse_atomic()) {
        return 1;
    }
    while (check_pos < tok_count && tokens[check_pos] && strcmp(tokens[check_pos], "|") == 0) {
        (check_pos)++;
        if (parse_atomic()) {
            return 1;
        }
    }
    return 0;
}

int parse_shell_cmd() {
    if (parse_cmd_grp()) {
        return 1;
    }

    while (check_pos < tok_count && tokens[check_pos] && 
           (strcmp(tokens[check_pos], "&") == 0 || strcmp(tokens[check_pos], ";") == 0)) {
        char *separator = tokens[check_pos];
        (check_pos)++;
        if (check_pos >= tok_count || !tokens[check_pos]) {
            if (strcmp(separator, "&") == 0) {
                return 0;
            } else {
               // printf("Invalid Syntax!\n");
                return 1;
            }
        }
        if (parse_cmd_grp()) {
            return 1;
        }
    }

    if (check_pos < tok_count && tokens[check_pos]) {
       // printf("Invalid Syntax!\n");
        return 1;
    }

    return 0;
}