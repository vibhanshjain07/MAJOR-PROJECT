#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "login_system.h"

void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

void read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buffer, (int)size, stdin) != NULL) {
        strip_newline(buffer);
    } else {
        buffer[0] = '\0';
    }
}

int is_strong_password(const char *password) {
    if (strlen(password) < 6) return 0;

    int has_digit = 0;
    for (size_t i = 0; password[i] != '\0'; ++i) {
        if (isdigit((unsigned char)password[i])) {
            has_digit = 1;
            break;
        }
    }
    return has_digit;
}

void register_user(void) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    printf("\n=== Register New User ===\n");
    read_line("Enter username (no spaces): ", username, sizeof(username));
    read_line("Enter password (no spaces, min 6 chars, must contain a digit): ",
              password, sizeof(password));

    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("Username or password cannot be empty.\n");
        return;
    }

    if (!is_strong_password(password)) {
        printf("Weak password. Use at least 6 characters and include a digit.\n");
        return;
    }

    FILE *fp_check = fopen(USERS_FILE, "r");
    if (fp_check != NULL) {
        char u[MAX_USERNAME], p[MAX_PASSWORD];
        while (fscanf(fp_check, "%49s %49s", u, p) == 2) {
            if (strcmp(u, username) == 0) {
                printf("Username '%s' already exists. Choose another.\n", username);
                fclose(fp_check);
                return;
            }
        }
        fclose(fp_check);
    }

    FILE *fp = fopen(USERS_FILE, "a");
    if (!fp) {
        perror("Error opening users file");
        return;
    }

    fprintf(fp, "%s %s\n", username, password);
    fclose(fp);

    printf("User '%s' registered successfully!\n", username);
}

void login_user(void) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    printf("\n=== User Login ===\n");
    read_line("Enter username: ", username, sizeof(username));
    read_line("Enter password: ", password, sizeof(password));

    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("No users registered yet. Please register first.\n");
        return;
    }

    char u[MAX_USERNAME], p[MAX_PASSWORD];
    int found = 0;

    while (fscanf(fp, "%49s %49s", u, p) == 2) {
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (found) {
        printf("\nLogin successful!\n");
        printf("Welcome, %s\n", username);
        printf("Your saved password is: %s\n", password);
    } else {
        printf("\nLogin failed. Invalid username or password.\n");
    }
}

void change_password(void) {
    char username[MAX_USERNAME];
    char old_password[MAX_PASSWORD];
    char new_password[MAX_PASSWORD];

    printf("\n=== Change Password ===\n");
    read_line("Enter username: ", username, sizeof(username));
    read_line("Enter current password: ", old_password, sizeof(old_password));

    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("No users registered yet. Please register first.\n");
        return;
    }

    User users[MAX_USERS];
    int count = 0;
    int index = -1;

    while (count < MAX_USERS &&
           fscanf(fp, "%49s %49s", users[count].username, users[count].password) == 2) {
        if (strcmp(users[count].username, username) == 0 &&
            strcmp(users[count].password, old_password) == 0) {
            index = count;
        }
        count++;
    }
    fclose(fp);

    if (index == -1) {
        printf("Username or password incorrect. Cannot change password.\n");
        return;
    }

    read_line("Enter new password (min 6 chars, must contain a digit): ",
              new_password, sizeof(new_password));

    if (!is_strong_password(new_password)) {
        printf("Weak password. Use at least 6 characters and include a digit.\n");
        return;
    }

    strcpy(users[index].password, new_password);

    fp = fopen(USERS_FILE, "w");
    if (!fp) {
        perror("Error opening users file for writing");
        return;
    }

    for (int i = 0; i < count; ++i) {
        fprintf(fp, "%s %s\n", users[i].username, users[i].password);
    }
    fclose(fp);

    printf("Password changed successfully for user '%s'.\n", username);
}

void show_all_users(void) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("No users registered yet.\n");
        return;
    }

    char u[MAX_USERNAME], p[MAX_PASSWORD];
    int count = 0;

    printf("\n=== List of Registered Users ===\n");
    while (fscanf(fp, "%49s %49s", u, p) == 2) {
        printf("%d. %s\n", ++count, u);
    }
    fclose(fp);

    if (count == 0) {
        printf("No users found.\n");
    }
}

void print_menu(void) {
    printf("\n===== Simple Login System =====\n");
    printf("1. Register new user\n");
    printf("2. Login\n");
    printf("3. Change password\n");
    printf("4. Show all users (usernames only)\n");
    printf("0. Exit\n");
    printf("Choose an option: ");
}

int main(void) {
    int running = 1;
    char line[32];

    printf("=== Basic Login System (C Project) ===\n");
    printf("Note: Credentials are stored in plain text in '%s'.\n\n", USERS_FILE);

    while (running) {
        print_menu();

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        int choice = atoi(line);

        switch (choice) {
            case 1:
                register_user();
                break;
            case 2:
                login_user();
                break;
            case 3:
                change_password();
                break;
            case 4:
                show_all_users();
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    }

    printf("Exiting program. Goodbye!\n");
    return 0;
}
