#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX 100
#define FILE_NAME "bank.dat"
#define LOG_FILE "transactions.txt"

struct Bank {
    char username[30];
    char password[30];
    float balance;
    int checkCode;
    float checkAmount;
};

struct Bank users[MAX];
int total = 0;

/* ========= INPUT ========= */

void cleanInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void getText(char *text, int size) {
    fgets(text, size, stdin);
    text[strcspn(text, "\n")] = '\0';
}

/* ========= FILE HANDLING ========= */

void loadData() {
    FILE *fp = fopen(FILE_NAME, "rb");
    if (!fp) return;

    fread(&total, sizeof(int), 1, fp);
    fread(users, sizeof(struct Bank), total, fp);
    fclose(fp);
}

void saveData() {
    FILE *fp = fopen(FILE_NAME, "wb");
    if (!fp) return;

    fwrite(&total, sizeof(int), 1, fp);
    fwrite(users, sizeof(struct Bank), total, fp);
    fclose(fp);
}

/* ========= LOGGING ========= */

void logTransaction(const char *user, const char *action, float amount, const char *extra) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(fp, "[%02d-%02d-%d %02d:%02d:%02d] %s | %s | Rs %.2f | %s\n",
        t->tm_mday, t->tm_mon+1, t->tm_year+1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        user, action, amount, extra);

    fclose(fp);
}

/* ========= DECLARATIONS ========= */

void createAccount();
int login();
void menu(int);
void deposit(int);
void withdraw(int);
void issueCheck(int);
void transfer(int);

/* ========= MAIN ========= */

int main() {
    int choice, id;
    loadData();   

    while(1) {
        printf("\n--- BANK ---");
        printf("\n1. Create Account");
        printf("\n2. Login");
        printf("\n3. Exit");
        printf("\nChoice: ");
        scanf("%d", &choice);
        cleanInput();

        switch(choice) {
            case 1: createAccount(); saveData(); break;
            case 2: id = login(); if(id != -1) menu(id); break;
            case 3: saveData(); exit(0);
            default: printf("Invalid!");
        }
    }
}

/* ========= CREATE ACCOUNT ========= */

void createAccount() {
    if (total >= MAX) return;

    printf("\nUsername: ");
    getText(users[total].username, 30);

    printf("Password: ");
    getText(users[total].password, 30);

    users[total].balance = 0;
    users[total].checkCode = 0;
    users[total].checkAmount = 0;

    logTransaction(users[total].username, "ACCOUNT CREATED", 0, "-");

    total++;
    printf("\nAccount created.\n");
}

/* ========= LOGIN ========= */

int login() {
    char u[30], p[30];
    printf("\nUsername: "); getText(u,30);
    printf("Password: "); getText(p,30);

    for(int i=0;i<total;i++)
        if(strcmp(users[i].username,u)==0 &&
           strcmp(users[i].password,p)==0) {
            printf("\nLogin OK.\n");
            return i;
        }

    printf("\nWrong login.\n");
    return -1;
}

/* ========= MENU ========= */

void menu(int id) {
    int ch;
    while(1) {
        printf("\n--- MENU ---");
        printf("\n1. Balance");
        printf("\n2. Deposit");
        printf("\n3. Withdraw");
        printf("\n4. Issue Check");
        printf("\n5. Transfer");
        printf("\n6. Logout");
        printf("\nSelect: ");
        scanf("%d",&ch);
        cleanInput();

        switch(ch) {
            case 1: printf("Balance: %.2f\n", users[id].balance); break;
            case 2: deposit(id); saveData(); break;
            case 3: withdraw(id); saveData(); break;
            case 4: issueCheck(id); saveData(); break;
            case 5: transfer(id); saveData(); break;
            case 6: return;
        }
    }
}

/* ========= DEPOSIT ========= */

void deposit(int id) {
    int type, code;
    float amt;

    printf("\n1. Cash\n2. Check\nChoice: ");
    scanf("%d",&type); cleanInput();

    if(type == 1) {
        printf("Amount: ");
        scanf("%f",&amt); cleanInput();
        users[id].balance += amt;
        logTransaction(users[id].username,"CASH DEPOSIT",amt,"-");
    }
    else if(type == 2) {
        printf("Check code: ");
        scanf("%d",&code); cleanInput();

        for(int i=0;i<total;i++) {
            if(users[i].checkCode == code) {
                users[id].balance += users[i].checkAmount;

                char info[60];
                sprintf(info, "FROM %s", users[i].username);
                logTransaction(users[id].username,
                    "CHECK DEPOSIT",
                    users[i].checkAmount,
                    info);

                users[i].checkCode = 0;
                users[i].checkAmount = 0;
                printf("\nCheck cleared.\n");
                return;
            }
        }
        printf("\nInvalid check.\n");
    }
}

/* ========= WITHDRAW ========= */

void withdraw(int id) {
    float amt;
    printf("Withdraw: ");
    scanf("%f",&amt); cleanInput();

    if(amt > users[id].balance) {
        logTransaction(users[id].username,"FAILED WITHDRAW",amt,"INSUFFICIENT");
        printf("Low balance.\n");
    }
    else {
        users[id].balance -= amt;
        logTransaction(users[id].username,"WITHDRAW",amt,"-");
        printf("Done.\n");
    }
}

/* ========= ISSUE CHECK ========= */

void issueCheck(int id) {
    float amt;
    int code = rand()%9000 + 1000;

    printf("Amount: ");
    scanf("%f",&amt); cleanInput();

    if(amt > users[id].balance) {
        logTransaction(users[id].username,"CHECK FAILED",amt,"LOW BALANCE");
        printf("Not enough.\n");
        return;
    }

    users[id].balance -= amt;
    users[id].checkCode = code;
    users[id].checkAmount = amt;

    logTransaction(users[id].username,"CHECK ISSUED",amt,"-");
    printf("Check Code: %d\n",code);
}

/* ========= TRANSFER ========= */

void transfer(int id) {
    char to[30];
    float amt;
    int found = -1;

    printf("Send to: ");
    getText(to,30);

    for(int i=0;i<total;i++)
        if(strcmp(users[i].username,to)==0) {
            found=i; break;
        }

    if(found == -1) return;

    printf("Amount: ");
    scanf("%f",&amt); cleanInput();

    if(amt > users[id].balance) {
        logTransaction(users[id].username,"TRANSFER FAILED",amt,"INSUFFICIENT");
        printf("Not enough.\n");
    }
    else {
        users[id].balance -= amt;
        users[found].balance += amt;

        char info[60];
        sprintf(info, "TO %s", users[found].username);

        logTransaction(users[id].username,"TRANSFER",amt,info);

        printf("Transferred.\n");
    }
}
