#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> // Include for better error reporting with errno

// --- Constants ---
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_SERVICE_NAME 100
#define MAX_USERS 100 // Maximum number of system users
#define USERS_FILE "users.txt"
#define CREDENTIALS_FILE "credentials.txt"

// --- Structures ---
typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} User;

typedef struct {
    char service_name[MAX_SERVICE_NAME];
    char stored_username[MAX_USERNAME];
    char stored_password[MAX_PASSWORD];
} Credential;

// --- Function Prototypes (for organization) ---
void strip_newline(char *s);
void read_line(const char *prompt, char *buffer, size_t size);
int is_strong_password(const char *password);
void register_user(void);
int login_user_internal(char *username_out);
void change_password(void);
void show_all_users(void);
void add_credential(void);
void view_credentials(void);
void search_credential(void);
void credential_manager_loop(void);
void print_main_menu(void);
void print_manager_menu(void);


// --- Utility Functions ---

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

// --- System User Functions ---

void register_user(void) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    printf("\n=== Register New User (for Credential Manager access) ===\n");
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

    // In a real system, you would hash the password here, not store it in plaintext.
    fprintf(fp, "%s %s\n", username, password);
    fclose(fp);

    printf("User '%s' registered successfully! You can now login.\n", username);
}

int login_user_internal(char *username_out) {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    read_line("Enter username: ", username, sizeof(username));
    read_line("Enter password: ", password, sizeof(password));

    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("No users registered yet.\n");
        return 0; // Failed
    }

    char u[MAX_USERNAME], p[MAX_PASSWORD];
    int found = 0;

    // Use loop to check credentials
    while (fscanf(fp, "%49s %49s", u, p) == 2) {
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (found) {
        // Copy the successfully logged-in username back
        strncpy(username_out, username, MAX_USERNAME - 1);
        username_out[MAX_USERNAME - 1] = '\0';
        return 1; // Success
    } else {
        printf("Login failed. Invalid username or password.\n");
        return 0; // Failed
    }
}

// Function to allow a user to change their system access password
void change_password(void) {
    char username[MAX_USERNAME];
    char old_password[MAX_PASSWORD];
    char new_password[MAX_PASSWORD];

    printf("\n=== Change System Password ===\n");
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

    // Read all users into the temporary array
    while (count < MAX_USERS &&
           fscanf(fp, "%49s %49s", users[count].username, users[count].password) == 2) {
        // Check if the current user/password matches the input
        if (strcmp(users[count].username, username) == 0 &&
            strcmp(users[count].password, old_password) == 0) {
            index = count;
        }
        count++;
    }
    fclose(fp);

    if (index == -1) {
        printf("Username or current password incorrect. Cannot change password.\n");
        return;
    }

    read_line("Enter new password (min 6 chars, must contain a digit): ",
              new_password, sizeof(new_password));

    if (!is_strong_password(new_password)) {
        printf("Weak password. Use at least 6 characters and include a digit.\n");
        return;
    }

    // Update the password in the temporary array
    strcpy(users[index].password, new_password);

    // Rewrite the entire file with the updated password
    fp = fopen(USERS_FILE, "w");
    if (!fp) {
        perror("Error opening users file for writing");
        return;
    }

    for (int i = 0; i < count; ++i) {
        fprintf(fp, "%s %s\n", users[i].username, users[i].password);
    }
    fclose(fp);

    printf("System password changed successfully for user '%s'.\n", username);
}

// Function to show all system users (Option 4)
void show_all_users(void) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) {
        printf("No users registered yet.\n");
        return;
    }

    char u[MAX_USERNAME], p[MAX_PASSWORD];
    int count = 0;

    printf("\n=== List of Credential Manager Users ===\n");
    // Read user/password pairs, but only print the username
    while (fscanf(fp, "%49s %49s", u, p) == 2) { 
        printf("%d. %s\n", ++count, u);
    }
    fclose(fp);

    if (count == 0) {
        printf("No users found.\n");
    }
}


// --- Credential Manager Functions ---

void add_credential(void) {
    Credential cred;
    printf("\n=== Add New Credential ===\n");

    read_line("Enter service name (e.g., 'Google', 'Bank'): ", cred.service_name, sizeof(cred.service_name));
    read_line("Enter username/email for this service: ", cred.stored_username, sizeof(cred.stored_username));
    read_line("Enter password for this service: ", cred.stored_password, sizeof(cred.stored_password));

    if (strlen(cred.service_name) == 0 || strlen(cred.stored_username) == 0 || strlen(cred.stored_password) == 0) {
        printf("All fields must be filled out.\n");
        return;
    }

    FILE *fp = fopen(CREDENTIALS_FILE, "a");
    if (!fp) {
        perror("Error opening credentials file");
        return;
    }

    // Store in the format: ServiceName StoredUsername StoredPassword
    fprintf(fp, "%s %s %s\n", cred.service_name, cred.stored_username, cred.stored_password);
    fclose(fp);

    printf("Credential for '%s' saved successfully!\n", cred.service_name);
}

void view_credentials(void) {
    FILE *fp = fopen(CREDENTIALS_FILE, "r");
    if (!fp) {
        if (errno == ENOENT) {
            printf("\nNo credentials saved yet in %s.\n", CREDENTIALS_FILE);
        } else {
            perror("Error opening credentials file");
        }
        return;
    }

    Credential cred;
    int count = 0;

    printf("\n=== Saved Credentials ===\n");
    printf("+----------------------+----------------------+----------------------+\n");
    printf("| %-20s | %-20s | %-20s |\n", "Service", "Username", "Password");
    printf("+----------------------+----------------------+----------------------+\n");

    // Use fscanf to read the three fields
    while (fscanf(fp, "%99s %49s %49s",
                  cred.service_name, cred.stored_username, cred.stored_password) == 3) {
        printf("| %-20s | %-20s | %-20s |\n",
               cred.service_name, cred.stored_username, cred.stored_password);
        count++;
    }
    printf("+----------------------+----------------------+----------------------+\n");

    fclose(fp);

    if (count == 0) {
        printf("No credentials found.\n");
    }
}

void search_credential(void) {
    char search_service[MAX_SERVICE_NAME];
    read_line("\nEnter the service name to search for: ", search_service, sizeof(search_service));

    FILE *fp = fopen(CREDENTIALS_FILE, "r");
    if (!fp) {
        printf("\nNo credentials saved yet.\n");
        return;
    }

    Credential cred;
    int found = 0;

    printf("\n=== Search Results ===\n");

    while (fscanf(fp, "%99s %49s %49s",
                  cred.service_name, cred.stored_username, cred.stored_password) == 3) {
        // Case-sensitive comparison
        if (strcmp(cred.service_name, search_service) == 0) {
            printf("Service: %s\n", cred.service_name);
            printf("Username: %s\n", cred.stored_username);
            printf("Password: %s\n", cred.stored_password);
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (!found) {
        printf("No credential found for service '%s'.\n", search_service);
    }
}


// --- Menu and Program Flow ---

void print_manager_menu(void) {
    printf("\n===== Credential Manager Menu =====\n");
    printf("1. Add New Credential\n");
    printf("2. View All Credentials\n");
    printf("3. Search Credential by Service Name\n");
    printf("4. Go back to Main Menu\n");
    printf("Choose an option: ");
}

void credential_manager_loop(void) {
    int running = 1;
    char line[32];

    while (running) {
        print_manager_menu();

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        int choice = atoi(line);

        switch (choice) {
            case 1:
                add_credential();
                break;
            case 2:
                view_credentials();
                break;
            case 3:
                search_credential();
                break;
            case 4:
                running = 0;
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    }
}

void print_main_menu(void) {
    printf("\n===== Simple Login/Credential System =====\n");
    printf("1. Register new user (System Access)\n");
    printf("2. Login to Credential Manager\n");
    printf("3. Change System Password\n");
    printf("4. Show all registered users\n");
    printf("0. Exit\n");
    printf("Choose an option: ");
}

int main(void) {
    int running = 1;
    char line[32];
    char current_user[MAX_USERNAME] = {0};

    printf("=== Basic Login System (C Project) ===\n");
    printf("⚠️ WARNING: Credentials are stored in plaintext in '%s' and '%s'.\n\n",
           USERS_FILE, CREDENTIALS_FILE);

    while (running) {
        print_main_menu();

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        int choice = atoi(line);

        switch (choice) {
            case 1:
                register_user();
                break;
            case 2:
                // Attempt to log in. If successful, enter the manager loop.
                printf("\n=== User Login ===\n");
                if (login_user_internal(current_user)) {
                    printf("\nLogin successful! Welcome, %s.\n", current_user);
                    credential_manager_loop();
                } else {
                    printf("\nLogin failed. Try again or register.\n");
                }
                break;
            case 3:
                // Call the fully implemented change_password function
                change_password(); 
                break;
            case 4:
                // Call the fully implemented show_all_users function
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