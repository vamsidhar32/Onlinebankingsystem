// constants.h
#define PORT 8081
#define Max_Size 50
#define Buf_Size 100
#define Authentication 1000
#define User 100
#define Joint 101
#define Admin 102
#define Deposit_Amount 1
#define Withdraw_Amount 2
#define Balance_Enquiry 3
#define Change_Password 4
#define View_Details 5
#define Exit 6

#define Sign_Up 7
#define Add_User 8
#define Delete_User 9
#define Modify_User 10
#define Search_User 11

#define SUCCESS 0
#define FAILURE -1
#define PRESENT 0
#define DELETED -1

// int login_flag=0;
struct User_account {
    char Name[Max_Size];
    char Password[Max_Size];
    float balance;
    int flag;
};

extern struct User_account login_user;

struct Admin_account {
    char Name[Max_Size];
    char Password[Max_Size];
};

extern struct Admin_account login_admin;

struct user {
    char Name[Max_Size];
    char Password[Max_Size];
};

extern struct user Joint_user;

struct Joint_account {
    int flag;
    int count;
    struct user holders[4];
    float balance;
};

extern struct flock lock;

int user_sign_in(char *Name, char *Password);
int User_Deposit_Amount(float amount);
int User_Withdraw_Amount(float amount);
float User_Balance_Enquiry();
int User_Change_Password();
void User_View_Details(int);
void User_lock(int fd, int n, int x);
int User_Unlock(int fd);

int joint_sign_in(char*, char*);
int Joint_Deposit_Amount(float);
int Joint_Withdraw_Amount(float);
float Joint_Balance_Enquiry();
int Joint_Change_Password(char*);
void Joint_View_Details(int);

int sign_up_admin(char*, char*);
int sign_in_admin(char*, char*);
int add_user(int, int);
int del_user(int, int);
int modify_user(int, int);
int search_user(int, int);
void Joint_lock(int, int, int);
int Joint_Unlock(int);
