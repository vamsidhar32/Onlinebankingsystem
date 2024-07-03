#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include "constants.h"


void req_handler(int sock);
// Global variables for login sessions
struct User_account login_user;
struct Admin_account login_admin;
struct user Joint_user;
struct flock lock;

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Starting Server\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) == -1) {
        perror("Listen failed");
        return 1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        int client_fd;
        socklen_t client_len = sizeof(client_addr);

        // Accept incoming connection
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("Accept failed");
            continue;  // Continue to next iteration of the loop
        }

        // Create a thread to handle client request
        pthread_t thread;
        if (pthread_create(&thread, NULL, (void *(*)(void *))req_handler, (void *)(intptr_t)client_fd) != 0) {
            perror("Thread creation failed");
            close(client_fd);
            continue;  // Continue to next iteration of the loop
        }

        // Detach thread so it can be automatically cleaned up after completion
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}

void req_handler(int sock) {
    char Name[Max_Size], Password[Max_Size];
    int choice, s, r;
    float amount;

    while (1) {
        read(sock, &choice, sizeof(int));
        read(sock, &s, sizeof(int));

        if (choice == Authentication) {
            read(sock, Name, sizeof(Name));
            read(sock, Password, sizeof(Password));
            
            if (s == User)
                r = user_sign_in(Name, Password);
            else if (s == Joint)
                r = joint_sign_in(Name, Password);
            else if (s == Admin)
                r = sign_in_admin(Name, Password);

            write(sock, &r, sizeof(r));
        }
        else if (choice == Deposit_Amount) {
            read(sock, &amount, sizeof(amount));
            
            if (s == User)
                r = User_Deposit_Amount(amount);
            else
                r = Joint_Deposit_Amount(amount);

            write(sock, &r, sizeof(r));
        }
        else if (choice == Withdraw_Amount) {
            read(sock, &amount, sizeof(amount));
            
            if (s == User)
                r = User_Withdraw_Amount(amount);
            else
                r = Joint_Withdraw_Amount(amount);

            write(sock, &r, sizeof(r));
        }
        else if (choice == Balance_Enquiry) {
            float x;
            
            if (s == User)
                x = User_Balance_Enquiry();
            else
                x = Joint_Balance_Enquiry();

            write(sock, &x, sizeof(x));
        }
        else if (choice == Change_Password) {
            char Password[Max_Size];
            read(sock, Password, sizeof(Password));
            
            if (s == User)
                r = User_Change_Password(Password);
            else
                r = Joint_Change_Password(Password);

            write(sock, &r, sizeof(r));
        }
        else if (choice == View_Details) {
            if (s == User)
                User_View_Details(sock);
            else
                Joint_View_Details(sock);
        }
        else if (choice == Sign_Up) {
            read(sock, Name, sizeof(Name));
            read(sock, Password, sizeof(Password));
            r = sign_up_admin(Name, Password);
            write(sock, &r, sizeof(r));
        }
        else if (choice == Add_User) {
            if (s == User)
                r = add_user(sock, User);
            else
                r = add_user(sock, Joint);

            write(sock, &r, sizeof(r));
        }
        else if (choice == Delete_User) {
            if (s == User)
                r = del_user(sock, User);
            else
                r = del_user(sock, Joint);

            write(sock, &r, sizeof(r));
        }
        else if (choice == Modify_User) {
            if (s == User)
                r = modify_user(sock, User);
            else
                r = modify_user(sock, Joint);

            write(sock, &r, sizeof(r));
        }
        else if (choice == Search_User) {
            if (s == User)
                r = search_user(sock, User);
            else
                r = search_user(sock, Joint);

            write(sock, &r, sizeof(r));
        }
        else if (choice == Exit) {
            break;
        }
    }

    close(sock);
}
