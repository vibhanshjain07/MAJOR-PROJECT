#ifndef LOGIN_SYSTEM_H
#define LOGIN_SYSTEM_H

#include <stdio.h>

#define USERS_FILE   "users.txt"
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_USERS    100

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} User;

void strip_newline(char *s);
void read_line(const char *prompt, char *buffer, size_t size);
int  is_strong_password(const char *password);

void register_user(void);
void login_user(void);
void change_password(void);
void show_all_users(void);
void print_menu(void);

#endif
