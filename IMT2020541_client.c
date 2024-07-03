#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "constants.h"

void interface(int sock);
int authentication(int sock, int type);
void user_operations(int sock, int type);
void admin_operations(int sock);

int main() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }
    printf("Starting Client\n");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  // Clear structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(client_fd);
        return 1;
    }

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_fd);
        return 1;
    }

    interface(client_fd);

    close(client_fd);  // Close the socket before exiting
    return 0;
}

void interface(int sock) {
    int choice;
    char Name[Max_Size];
    char Password[Max_Size];

    while (1) {
        printf("Interface Menu:\n");
        printf("1. Sign up admin\n");
        printf("2. Login as user\n");
        printf("3. Login as joint user\n");
        printf("4. Login as admin\n");
        printf("5. Exit\n\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline character

        switch (choice) {
            case 1:
                printf("Enter admin name to create: ");
                fgets(Name, Max_Size, stdin);
                printf("Enter password: ");
                fgets(Password, Max_Size, stdin);
                Name[strcspn(Name, "\n")] = '\0';  // Remove newline from fgets input
                Password[strcspn(Password, "\n")] = '\0';
                int option = Sign_Up;
                write(sock, &option, sizeof(option));
                option = Admin;
                write(sock, &option, sizeof(option));
                write(sock, Name, Max_Size);
                write(sock, Password, Max_Size);
                read(sock, &option, sizeof(option));
                if (option == SUCCESS)
                    printf("New admin added successfully.\n");
                else
                    printf("Username already exists. Please try again.\n");
                break;
            
            case 2:
                if (authentication(sock, User) == SUCCESS) {
                    user_operations(sock, User);
                    printf("User logout successful.\n");
                }
                break;
            
            case 3:
                if (authentication(sock, Joint) == SUCCESS) {
                    user_operations(sock, Joint);
                    printf("Joint user logout successful.\n");
                }
                break;
            
            case 4:
                // Admin login is handled directly in admin_operations
                admin_operations(sock);
                printf("Admin logout successful.\n");
                break;
            
            case 5:
                exit(0);
            
            default:
                printf("Invalid input. Please enter a valid option.\n\n");
                break;
        }
    }
}

int authentication(int sock, int type) {
    int status = Authentication;
    char Name[Max_Size];
    char Password[Max_Size];

    printf("Enter username: ");
    fgets(Name, Max_Size, stdin);
    printf("Enter password: ");
    fgets(Password, Max_Size, stdin);
    Name[strcspn(Name, "\n")] = '\0';  // Remove newline from fgets input
    Password[strcspn(Password, "\n")] = '\0';

    // Send authentication details to the server
    write(sock, &status, sizeof(status));
    write(sock, &type, sizeof(type));
    write(sock, Name, Max_Size);
    write(sock, Password, Max_Size);

    // Receive authentication result from the server
    read(sock, &status, sizeof(status));

    if (status == SUCCESS)
        printf("\nLogin successful.\n\n");
    else
        printf("\nLogin unsuccessful. Please try again.\n\n");

    return status;
}
void user_operations(int sock, int t) {
    int choice, option, ret_val, s, count;
    float amount;
    char Name[Max_Size], Password[Max_Size], return_str[Buf_Size];

    while (1) {
        printf("Single User/Joint User Menu:\n");
        printf("1. Deposit Amount\n");
        printf("2. Withdraw Amount\n");
        printf("3. Balance Enquiry\n");
        printf("4. Change Password\n");
        printf("5. View Details\n");
        printf("6. To logout/Exit\n\n");
        printf("Enter the choice:");
        scanf("%d", &choice);
        getchar();  // Consume newline left in buffer by scanf

        switch (choice) {
            case 1:
                printf("Enter the amount to deposit:");
                scanf("%f", &amount);
                option = Deposit_Amount;
                break;

            case 2:
                printf("Enter the amount to withdraw:");
                scanf("%f", &amount);
                option = Withdraw_Amount;
                break;

            case 3:
                option = Balance_Enquiry;
                break;

            case 4:
                option = Change_Password;
                printf("Enter a new password:");
                fgets(Password, Max_Size, stdin);  // Consider using fgets instead of scanf for consistency
                break;

            case 5:
                option = View_Details;
                break;

            case 6:
                option = Exit;
                t = User;
                break;

            default:
                printf("Invalid input, please enter a valid input...\n");
                continue;  // Restart the loop to get valid input
        }

        // Send operation choice and relevant data to the server
        write(sock, &option, sizeof(option));
        if (t == User)
            s = User;
        else
            s = Joint;
        write(sock, &s, sizeof(s));

        switch (choice) {
            case 1:  // Deposit Amount
            case 2:  // Withdraw Amount
                write(sock, &amount, sizeof(amount));
                read(sock, &ret_val, sizeof(ret_val));
                if (choice == 1 && ret_val == SUCCESS)
                    printf("\nAmount deposited successfully...\n\n");
                else if (choice == 2 && ret_val == SUCCESS)
                    printf("\nAmount withdrawn successfully...\n\n");
                else if (choice == 2 && ret_val == FAILURE)
                    printf("\nInsufficient balance...\n\n");
                break;

            case 3:  // Balance Enquiry
                read(sock, &amount, sizeof(amount));
                printf("Remaining Balance: %f\n\n", amount);
                break;

            case 4:  // Change Password
                write(sock, Password, sizeof(Password));
                read(sock, &ret_val, sizeof(ret_val));
                if (ret_val == SUCCESS)
                    printf("\nNew Password updated successfully...\n\n");
                else
                    printf("\nPassword update failed, try again...\n\n");
                break;

            case 5:  // View Details
                read(sock, &ret_val, sizeof(ret_val));
                if (ret_val == 1) {
                    if (t == User) {
                        read(sock, &amount, sizeof(amount));
                        read(sock, Name, sizeof(Name));
                        printf("\nUser Name: %s\nType: Single User\nRemaining Balance: %f\n\n", Name, amount);
                    } else {
                        read(sock, &count, sizeof(count));
                        printf("\nNo of users: %d\nType: Joint account\n", count);
                        for (int i = 0; i < count; i++) {
                            read(sock, Name, sizeof(Name));
                            printf("User Name: %s\n", Name);
                        }
                        read(sock, &amount, sizeof(amount));
                        printf("Remaining Balance: %f\n\n", amount);
                    }
                } else {
                    printf("\nUnable to fetch details...\n\n");
                }
                break;

            case 6:  // Exit
                write(sock, &t, sizeof(t));
                return;

            default:
                printf("Invalid input, please enter a valid input...\n");
        }
    }
}

void admin_operations(int sock) {
    char Name[Max_Size], Passsword[Max_Size];
    int choice, r, count, option, t;
    float balance;

    while (1) {
        printf("Single User/Joint User Menu:\n");
        printf("1. Add new single account\n");
        printf("2. Add new Joint account\n");
        printf("3. Delete single account\n");
        printf("4. Delete joint account\n");
        printf("5. Modify username of single account\n");
        printf("6. Modify username of Joint account\n");
        printf("7. Search single account\n");
        printf("8. Search Joint account\n");
        printf("9. Exit/Logout\n");
        printf("Enter the choice:");
        scanf("%d", &choice);
        getchar();  // Consume newline left in buffer by scanf

        switch (choice) {
            case 1:
                option = Add_User;
                write(sock, &option, sizeof(option));
                t = User;
                write(sock, &t, sizeof(t));
                printf("Enter the new single account User name:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                printf("Enter Password for this new user:");
                fgets(Passsword, Max_Size, stdin);
                write(sock, Passsword, sizeof(Passsword));
                printf("Enter balance:\n");
                scanf("%f", &balance);
                write(sock, &balance, sizeof(balance));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                    printf("Single user added successfully...\n");
                else
                    printf("Failed to add single user...\n");
                break;

            case 2:
                option = Add_User;
                write(sock, &option, sizeof(option));
                t = Joint;
                write(sock, &t, sizeof(t));
                printf("Enter the number of users in joint account (min=1 and max=4):");
                scanf("%d", &count);
                getchar();  // Consume newline left in buffer by scanf
                write(sock, &count, sizeof(count));
                for (int i = 0; i < count; i++) {
                    printf("Enter the username for new user:");
                    fgets(Name, Max_Size, stdin);
                    write(sock, Name, sizeof(Name));
                    printf("Enter Password for this new user:");
                    fgets(Passsword, Max_Size, stdin);
                    write(sock, Passsword, sizeof(Passsword));
                }
                printf("Enter balance:\n");
                scanf("%f", &balance);
                write(sock, &balance, sizeof(balance));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                    printf("Joint user added successfully...\n");
                else
                    printf("Failed to add joint user...\n");
                break;

            case 3:
                option = Delete_User;
                write(sock, &option, sizeof(option));
                t = User;
                write(sock, &t, sizeof(t));
                printf("Enter the username to delete:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                    printf("Single user deleted successfully...\n");
                else
                    printf("Single user with this username does not exist...\n");
                break;

            case 4:
                option = Delete_User;
                write(sock, &option, sizeof(option));
                t = Joint;
                write(sock, &t, sizeof(t));
                printf("Enter the username of joint user to delete:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                    printf("Joint user deleted successfully...\n");
                else
                    printf("Joint user with this username does not exist...\n");
                break;

            case 5:
                option = Modify_User;
                write(sock, &option, sizeof(option));
                t = User;
                write(sock, &t, sizeof(t));
                printf("Enter the username to modify:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                printf("Enter the new username:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                    printf("Single account username modified successfully...\n");
                else
                    printf("Failed to modify single account username...\n");
                break;

            case 6:
                option = Modify_User;
                write(sock, &option, sizeof(option));
                t = Joint;
                write(sock, &t, sizeof(t));
                printf("Enter the username to modify:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                printf("Enter the new username:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                    printf("Joint account username modified successfully...\n");
                else
                    printf("Failed to modify joint account username...\n");
                break;

            case 7:
                option = Search_User;
                write(sock, &option, sizeof(option));
                t = User;
                write(sock, &t, sizeof(t));
                printf("Enter the username to search:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                printf("Single account with this username is present...\n");
                else
                printf("Single account with this username is not present...\n");
                break;
                
                
            case 8:
                option = Search_User;
                write(sock, &option, sizeof(option));
                t = Joint;
                write(sock, &t, sizeof(t));
                printf("Enter the username to search:");
                fgets(Name, Max_Size, stdin);
                write(sock, Name, sizeof(Name));
                read(sock, &r, sizeof(r));
                if (r == SUCCESS)
                    printf("Joint account with this username is present...\n");
                else
                    printf("Joint account with this username is not present...\n");
                break;

            case 9:
                option = Exit;
                write(sock, &option, sizeof(option));
                t = Admin;
                write(sock, &t, sizeof(t));
                return;
                break;

            default:
                printf("Invalid input, please enter a valid input...\n");
        }
    }
}