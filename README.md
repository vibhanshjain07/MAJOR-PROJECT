# Basic Login System in C

This project implements a simple console-based login system in C.  
Users can register, log in, change their password, and list all registered usernames.  
User credentials are stored in a plain text file `users.txt` in the same directory as the executable.

## Features

- Register new user with a basic password strength check
- User login with username and password
- Change password after verifying current password
- Display all registered usernames
- Simple text-file based storage

## How to Compile

gcc -Wall -Wextra -std=c11 -Iinclude src/main.c -o login_system


## How to Run

./login_system


Ensure the program has permission to create and modify `users.txt` in the current directory.

## Sample Input

A sample interactive session is provided in `sampleinput.txt` for automated testing.
