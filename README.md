# Online Banking Management System

This project implements a multi-threaded server for a banking system, which supports various operations like user authentication, account management, balance enquiry, deposits, withdrawals, and administrative actions.

## Features

- **User Authentication**: Sign in for users, joint account holders, and admins.
- **Account Management**: Add, delete, modify, and search for user accounts.
- **Transactions**: Deposit and withdraw amounts from user accounts.
- **Administrative Actions**: Admins can manage user accounts and view details.

## Directory Structure

├── IMT2020541_server.c <br>
├── IMT2020541_client.c <br>
├── user_functions.c    <br>
├── joint_functions.c   <br>
├── admin_functions.c   <br>
├── makefile            <br>
└── README.md


## Files

- `constants.h`: Contains macro definitions and structure declarations.
- `IMT2020134_server.c`: Main server implementation file.
- `user_functions.c`: Functions for user account management.
- `joint_functions.c`: Functions for joint account management.
- `admin_functions.c`: Functions for admin account management.
- `makefile`: Makefile to compile the project.
- `README.md`: This file.

## Prerequisites

- GCC (GNU Compiler Collection)
- pthread library

## Compilation

To compile the project, follow these steps:
1. Compile the server:
```
make server
```
This will create an executable named server.

2. Run the server:
```
./server
```
The server will start listening on the specified port for client connections.

3. Compile the client:
```
make client
```
This will create an executable named client.

4. Run the client:
```
./client
```

Client Menu
After starting the client program (./client), you will see a menu where you can select various options to interact with the server:

+ Choose an authentication option (User, Joint, Admin).
+ Depending on the authentication type:

+ For Single User/Joint User Menu:
   1. Deposit Amount
   2. Withdraw Amount
   3. Balance Enquiry
   4. Change Password
   5. View Details
   6. Logout/Exit
+ For Admin Menu:
   1. Add new single account
   2. Add new Joint account
   3. Delete single account
   4. Delete joint account
   5. Modify username of single account
   6. Modify username of Joint account
   7. Search single account
   8. Search Joint account
   9. Exit/Logout


Follow the prompts in the client program to perform these actions. Ensure the server (./server) is running and listening for these requests while interacting with the client menu.

Project Components
* Server (IMT2020134_server.c)
The main server file sets up a socket, binds it to a port, and listens for incoming connections. Each connection is handled in a separate thread by the req_handler function.

* User Functions (user_functions.c)
Contains functions related to user account operations like signing in, depositing, withdrawing, balance enquiry, changing password, and viewing details.

* Joint Functions (joint_functions.c)
Contains functions related to joint account operations similar to the user functions but for joint accounts.

* Admin Functions (admin_functions.c)
Contains functions for admin operations such as signing in, adding users, deleting users, modifying users, and searching for users.

<!-- Function Descriptions
* void req_handler(int sock);
Handles client requests based on the operation code received from the client. Supports operations like authentication, deposits, withdrawals, balance enquiry, password changes, viewing details, user management, etc.

Authentication
+ int sign_in_admin(char *Name, char *Password);
+ int user_sign_in(char *Name, char *Password);
+ int joint_sign_in(char *Name, char *Password);

User Management
+ int add_user(int sock, int t);
+ int del_user(int sock, int t);
+ int modify_user(int sock, int t);
+ int search_user(int sock, int t);

Transactions
+ int User_Deposit_Amount(float amount);
+ int Joint_Deposit_Amount(float amount);
+ int User_Withdraw_Amount(float amount);
+ int Joint_Withdraw_Amount(float amount);

Balance Enquiry
+ float User_Balance_Enquiry();
+ float Joint_Balance_Enquiry();

Password Management
+ int User_Change_Password(char *Password);
+ int Joint_Change_Password(char *Password);

View Details
+ void User_View_Details(int sock);
+ void Joint_View_Details(int sock);int sign_in_admin(char *Name, char *Password);
+ int user_sign_in(char *Name, char *Password);
+ int joint_sign_in(char *Name, char *Password);

User Management
+ int add_user(int sock, int t);
+ int del_user(int sock, int t);
+ int modify_user(int sock, int t);
+ int search_user(int sock, int t);

Transactions
+ int User_Deposit_Amount(float amount);
+ int Joint_Deposit_Amount(float amount);
+ int User_Withdraw_Amount(float amount);
+ int Joint_Withdraw_Amount(float amount);

Balance Enquiry
+ float User_Balance_Enquiry();
+ float Joint_Balance_Enquiry();

Password Management
+ int User_Change_Password(char *Password);
+ int Joint_Change_Password(char *Password);

View Details
+ void User_View_Details(int sock);
+ void Joint_View_Details(int sock);  -->