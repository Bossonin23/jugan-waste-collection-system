#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

// Simple URL decode
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if (*src == '%' && (a = src[1]) && (b = src[2]) && isxdigit(a) && isxdigit(b)) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Parse POST data into a map (simple key-value)
char* get_param(char *data, const char *key) {
    char *ptr = strstr(data, key);
    if (!ptr) return NULL;
    ptr += strlen(key) + 1; // Skip key=
    char *end = strchr(ptr, '&');
    if (!end) end = ptr + strlen(ptr);
    int len = end - ptr;
    char *value = malloc(len + 1);
    strncpy(value, ptr, len);
    value[len] = '\0';
    url_decode(value, value);
    return value;
}

// Load users from file
void load_users(char ***users, int *count) {
    FILE *file = fopen("users.txt", "r");
    if (!file) return;
    char line[256];
    *count = 0;
    while (fgets(line, sizeof(line), file)) {
        *users = realloc(*users, (*count + 1) * sizeof(char*));
        (*users)[*count] = strdup(line);
        (*count)++;
    }
    fclose(file);
}

// Save users to file
void save_users(char **users, int count) {
    FILE *file = fopen("users.txt", "w");
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s", users[i]);
    }
    fclose(file);
}

// Similar for announcements and suggestions (omitted for brevity, but implement similarly)

// Main CGI handler
int main() {
    char *method = getenv("REQUEST_METHOD");
    if (!method || strcmp(method, "POST") != 0) {
        printf("Content-Type: application/json\n\n{\"error\": \"POST required\"}");
        return 1;
    }

    char *content_length_str = getenv("CONTENT_LENGTH");
    if (!content_length_str) {
        printf("Content-Type: application/json\n\n{\"error\": \"Missing CONTENT_LENGTH\"}");
        return 1;
    }
    int content_length = atoi(content_length_str);
    if (content_length > 10000) {
        printf("Content-Type: application/json\n\n{\"error\": \"Data too large\"}");
        return 1;
    }

    char *post_data = malloc(content_length + 1);
    fread(post_data, 1, content_length, stdin);
    post_data[content_length] = '\0';

    char *action = get_param(post_data, "action");
    if (!action) {
        printf("Content-Type: application/json\n\n{\"error\": \"Missing action\"}");
        free(post_data);
        return 1;
    }

    // Handle actions
    if (strcmp(action, "login") == 0) {
        char *username = get_param(post_data, "username");
        char *password = get_param(post_data, "password");
        // Load users and check
        char **users = NULL;
        int count = 0;
        load_users(&users, &count);
        int found = 0;
        char role[10] = "user";
        for (int i = 0; i < count; i++) {
            char u[50], p[50], r[10];
            sscanf(users[i], "%s %s %s", u, p, r);
            if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
                strcpy(role, r);
                found = 1;
                break;
            }
        }
        if (found) {
            printf("Content-Type: application/json\n\n{\"success\": true, \"user\": {\"username\": \"%s\", \"role\": \"%s\"}}", username, role);
        } else {
            printf("Content-Type: application/json\n\n{\"success\": false, \"message\": \"Invalid credentials\"}");
        }
        for (int i = 0; i < count; i++) free(users[i]);
        free(users);
    } else if (strcmp(action, "signup") == 0) {
        // Similar logic: check if username exists, add if not
        // Return success or error
    } else if (strcmp(action, "get_announcements") == 0) {
        // Load and return announcements as JSON array
    } else if (strcmp(action, "post_announcement") == 0) {
        // Add announcement to file
    } // Add similar for edit/delete announcements, suggestions, user management

    free(post_data);
    free(action);
    return 0;
}