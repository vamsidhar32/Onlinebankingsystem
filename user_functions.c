#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "constants.h"

extern struct User_account login_user;
extern struct flock lock;

int user_sign_in(char *Name, char *Password) {
    int count = 0;
    struct User_account *u = malloc(sizeof(struct User_account));
    if (u == NULL) {
        perror("Memory allocation failed");
        return FAILURE;
    }

    int fd = open("user.txt", O_CREAT | O_RDWR, 0744);
    if (fd == -1) {
        perror("Error opening user.txt");
        free(u);
        return FAILURE;
    }

    while (read(fd, u, sizeof(struct User_account))) {
        count++;
        if (u->flag == PRESENT) {
            if (!strcmp(u->Name, Name) && !strcmp(u->Password, Password)) {
                User_lock(fd, count, 0);
                login_user = *u;
                User_Unlock(fd);
                close(fd);
                free(u);
                return SUCCESS;
            }
        }
    }

    close(fd);
    free(u);
    return FAILURE;
}

int User_Deposit_Amount(float amount) {
    int count = 0;
    int fd = open("user.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening user.txt");
        return FAILURE;
    }

    struct User_account *u = malloc(sizeof(struct User_account));
    if (u == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return FAILURE;
    }

    while (read(fd, u, sizeof(struct User_account))) {
        count++;
        if (u->flag == PRESENT && !strcmp(login_user.Name, u->Name)) {
            User_lock(fd, count, 1);
            u->balance += amount;
            lseek(fd, -sizeof(struct User_account), SEEK_CUR);
            if (write(fd, u, sizeof(struct User_account)) == -1) {
                perror("Error writing to user.txt");
                User_Unlock(fd);
                close(fd);
                free(u);
                return FAILURE;
            }
            User_Unlock(fd);
            close(fd);
            free(u);
            return SUCCESS;
        }
    }

    close(fd);
    free(u);
    return FAILURE;
}

int User_Withdraw_Amount(float amount) {
    int count = 0;
    int fd = open("user.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening user.txt");
        return FAILURE;
    }

    struct User_account *u = malloc(sizeof(struct User_account));
    if (u == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return FAILURE;
    }

    while (read(fd, u, sizeof(struct User_account))) {
        count++;
        if (u->flag == PRESENT && !strcmp(login_user.Name, u->Name)) {
            User_lock(fd, count, 1);
            if (u->balance >= amount) {
                u->balance -= amount;
                lseek(fd, -sizeof(struct User_account), SEEK_CUR);
                if (write(fd, u, sizeof(struct User_account)) == -1) {
                    perror("Error writing to user.txt");
                    User_Unlock(fd);
                    close(fd);
                    free(u);
                    return FAILURE;
                }
                User_Unlock(fd);
                close(fd);
                free(u);
                return SUCCESS;
            } else {
                User_Unlock(fd);
                close(fd);
                free(u);
                return FAILURE;
            }
        }
    }

    close(fd);
    free(u);
    return FAILURE;
}

float User_Balance_Enquiry() {
    int count = 0;
    float balance = -1;
    int fd = open("user.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening user.txt");
        return -1;
    }

    struct User_account *u = malloc(sizeof(struct User_account));
    if (u == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return -1;
    }

    while (read(fd, u, sizeof(struct User_account))) {
        count++;
        if (u->flag == PRESENT && !strcmp(login_user.Name, u->Name)) {
            User_lock(fd, count, 0);
            balance = u->balance;
            printf("Balance: %f\n", u->balance);
            User_Unlock(fd);
            close(fd);
            free(u);
            return balance;
        }
    }

    close(fd);
    free(u);
    return -1;
}

int User_Change_Password(char *Password) {
    int count = 0;
    int fd = open("user.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening user.txt");
        return FAILURE;
    }

    struct User_account *u = malloc(sizeof(struct User_account));
    if (u == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return FAILURE;
    }

    while (read(fd, u, sizeof(struct User_account))) {
        count++;
        if (u->flag == PRESENT && !strcmp(u->Name, login_user.Name)) {
            User_lock(fd, count, 1);
            strcpy(u->Password, Password);
            strcpy(login_user.Password, Password);
            lseek(fd, -sizeof(struct User_account), SEEK_CUR);
            if (write(fd, u, sizeof(struct User_account)) == -1) {
                perror("Error writing to user.txt");
                User_Unlock(fd);
                close(fd);
                free(u);
                return FAILURE;
            }
            User_Unlock(fd);
            close(fd);
            free(u);
            return SUCCESS;
        }
    }

    close(fd);
    free(u);
    return FAILURE;
}

void User_View_Details(int sock) {
    int i = 0;
    char *return_str = malloc(sizeof(char) * Buf_Size);
    if (return_str == NULL) {
        perror("Memory allocation failed");
        return;
    }

    int count = 0;
    int fd = open("user.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening user.txt");
        free(return_str);
        return;
    }

    struct User_account *u = malloc(sizeof(struct User_account));
    if (u == NULL) {
        perror("Memory allocation failed");
        close(fd);
        free(return_str);
        return;
    }

    while (read(fd, u, sizeof(struct User_account))) {
        count++;
        if (u->flag == PRESENT && !strcmp(login_user.Name, u->Name)) {
            i = 1;
            write(sock, &i, sizeof(int));
            User_lock(fd, count, 0);
            write(sock, &u->balance, sizeof(u->balance));
            write(sock, u->Name, sizeof(u->Name));
            User_Unlock(fd);
            close(fd);
            free(u);
            free(return_str);
            return;
        }
    }

    write(sock, &i, sizeof(i));
    close(fd);
    free(u);
    free(return_str);
}

void User_lock(int fd, int n, int x) {
    lock.l_type = (x == 0) ? F_RDLCK : F_WRLCK;
    lock.l_len = sizeof(struct User_account);
    lock.l_start = (n - 1) * sizeof(struct User_account);
    lock.l_pid = getpid();
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("Error locking file");
    }
}

int User_Unlock(int fd) {
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error unlocking file");
        return FAILURE;
    }
    return SUCCESS;
}
