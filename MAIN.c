#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_USERS 50
#define STACK_SIZE 5
#define QUEUE_SIZE 10

typedef struct {
    char username[30];
    char password[30];
    char name[40];
    int balance;
} User;
typedef struct {
    char type[15]; 
    int amt;
    int oldBalance;
    char otherUser[30];
} UndoOp;
typedef struct {
    UndoOp ops[STACK_SIZE];
    int top;
} Stack;
typedef struct {
    char user[30];
    int amount;
} Loan;
typedef struct {
    Loan items[QUEUE_SIZE];
    int front, rear, count;
} Queue;
User users[MAX_USERS];
int userCount = 0;
User *currentUser = NULL;
Stack undoStack;
Queue loanQueue;
void clearInput() {
    while(getchar() != '\n');
}
User* findUser(char *username) {
    int i;
    for(i=0; i<userCount; i++) {
        if(strcmp(users[i].username, username)==0)
            return &users[i];
    }
    return NULL;
}
void initStack() {
    undoStack.top = -1;
}
void pushOp(char *type, int amount, int prevBal, char *other) {
    if(undoStack.top >= STACK_SIZE-1) {
        int j;
        for(j=0; j<STACK_SIZE-1; j++)
            undoStack.ops[j] = undoStack.ops[j+1];
        undoStack.top--;
    }
    undoStack.top++;
    strcpy(undoStack.ops[undoStack.top].type, type);
    undoStack.ops[undoStack.top].amt = amount;
    undoStack.ops[undoStack.top].oldBalance = prevBal;
    if(other != NULL)
        strcpy(undoStack.ops[undoStack.top].otherUser, other);
    else
        strcpy(undoStack.ops[undoStack.top].otherUser, "");
}
void initQueue() {
    loanQueue.front = 0;
    loanQueue.rear = -1;
    loanQueue.count = 0;
}
void addLoan(char *username, int amt) {
    if(loanQueue.count >= QUEUE_SIZE) {
        printf("Loan queue full!\n");
        return;
    }
    loanQueue.rear = (loanQueue.rear + 1) % QUEUE_SIZE;
    strcpy(loanQueue.items[loanQueue.rear].user, username);
    loanQueue.items[loanQueue.rear].amount = amt;
    loanQueue.count++;
}
Loan removeLoan() {
    Loan l = {"", 0};
    if(loanQueue.count == 0) return l;
    l = loanQueue.items[loanQueue.front];
    loanQueue.front = (loanQueue.front + 1) % QUEUE_SIZE;
    loanQueue.count--;
    return l;
}
void createAccount() {
    if(userCount >= MAX_USERS) {
        printf("Cannot create more accounts\n");
        return;
    }
    User newUser;
    printf("\nEnter name: ");
    scanf("%39s", newUser.name);
    printf("Choose username: ");
    scanf("%29s", newUser.username);
    if(findUser(newUser.username)) {
        printf("Username taken!\n");
        return;
    }
    printf("Set password: ");
    scanf("%29s", newUser.password);
    newUser.balance = 0;
    users[userCount++] = newUser;
    printf("Account created successfully!\n");
}
void deposit() {
    int amt;
    printf("\nAmount: ");
    scanf("%d", &amt);
    if(amt <= 0) {
        printf("Invalid amount\n");
        return;
    }
    pushOp("deposit", amt, currentUser->balance, NULL);
    currentUser->balance += amt;
    printf("Deposited %d. New balance: %d\n", amt, currentUser->balance);
}
void transfer() {
    char toUsername[30];
    int amt;
    printf("\nRecipient username: ");
    scanf("%29s", toUsername);
    User *recipient = findUser(toUsername);
    if(!recipient) {
        printf("User not found\n");
        return;
    }
    if(strcmp(toUsername, currentUser->username) == 0) {
        printf("Can't transfer to yourself\n");
        return;
    }
    printf("Amount: ");
    scanf("%d", &amt);
    if(amt <= 0 || amt > currentUser->balance) {
        printf("Invalid amount\n");
        return;
    }
    pushOp("transfer", amt, currentUser->balance, toUsername);
    currentUser->balance -= amt;
    recipient->balance += amt;
    printf("Transferred %d to %s\n", amt, toUsername);
}
void undoLast() {
    if(undoStack.top < 0) {
        printf("\nNothing to undo\n");
        return;
    }
    UndoOp op = undoStack.ops[undoStack.top--];
    if(strcmp(op.type, "deposit") == 0) {
        currentUser->balance = op.oldBalance;
        printf("Undone: Deposit of %d\n", op.amt);
    }
    else if(strcmp(op.type, "transfer") == 0) {
        User *other = findUser(op.otherUser);
        if(other) {
            currentUser->balance = op.oldBalance;
            other->balance -= op.amt;
            printf("Undone: Transfer of %d to %s\n", op.amt, op.otherUser);
        }
    }
    printf("Balance: %d\n", currentUser->balance);
}
void showHistory() {
    if(undoStack.top < 0) {
        printf("\nNo recent activity\n");
        return;
    }
    printf("\n--- Recent Activity ---\n");
    int i;
    for(i = undoStack.top; i >= 0; i--) {
        UndoOp op = undoStack.ops[i];
        if(strcmp(op.type, "deposit") == 0)
            printf("%d. Deposited %d\n", undoStack.top-i+1, op.amt);
        else
            printf("%d. Sent %d to %s\n", undoStack.top-i+1, op.amt, op.otherUser);
    }
}
void requestLoan() {
    int amt;
    printf("\nLoan amount: ");
    scanf("%d", &amt);
    if(amt <= 0) {
        printf("Invalid amount\n");
        return;
    }
    addLoan(currentUser->username, amt);
    printf("Loan requested! Position in queue: %d\n", loanQueue.count);
}
void checkLoanStatus() {
    if(loanQueue.count == 0) {
        printf("\nNo pending loans\n");
        return;
    }
    int pos = 0, found = 0;
    int i;
    for(i=0; i<loanQueue.count; i++) {
        int idx = (loanQueue.front + i) % QUEUE_SIZE;
        if(strcmp(loanQueue.items[idx].user, currentUser->username) == 0) {
            pos = i+1;
            found = 1;
            break;
        }
    }
    
    if(found)
        printf("\nYour loan is at position: %d\n", pos);
    else
        printf("\nNo loan request found\n");
}
void processLoans() {
    if(loanQueue.count == 0) {
        printf("\nNo loans to process\n");
        return;
    }
    printf("\n--- Processing Loans (Total: %d) ---\n", loanQueue.count);
    while(loanQueue.count > 0) {
        Loan ln = loanQueue.items[loanQueue.front];
        printf("\nUser: %s, Amount: %d\n", ln.user, ln.amount);
        printf("1-Approve  2-Reject  3-Stop: ");

        int choice;
        scanf("%d", &choice);
        
        if(choice == 1) {
            User *u = findUser(ln.user);
            if(u) {
                u->balance += ln.amount;
                printf("Approved! %d credited\n", ln.amount);
            }
            removeLoan();
        }
        else if(choice == 2) {
            printf("Rejected\n");
            removeLoan();
        }
        else
            break;
    }
}

void userMenu() {
    int choice;
    while(currentUser != NULL) {
        printf("\n--- Menu ---\n");
        printf("1.Balance 2.Deposit 3.Transfer 4.Undo\n");
        printf("5.History 6.Loan 7.Status 8.Process 9.Logout\n");
        printf("Choice: ");
        scanf("%d", &choice);
        switch(choice) {
            case 1:
                printf("\nBalance: %d\n", currentUser->balance);
                break;
            case 2: deposit(); break;
            case 3: transfer(); break;
            case 4: undoLast(); break;
            case 5: showHistory(); break;
            case 6: requestLoan(); break;
            case 7: checkLoanStatus(); break;
            case 8: processLoans(); break;
            case 9:
                printf("Logged out\n");
                currentUser = NULL;
                initStack();
                break;
            default:
                printf("Invalid\n");
        }
    }
}

void login() {
    char username[30], password[30];
    printf("\nUsername: ");
    scanf("%29s", username);
    printf("Password: ");
    scanf("%29s", password);
    
    User *u = findUser(username);
    if(u == NULL || strcmp(u->password, password) != 0) {
        printf("Invalid credentials\n");
        return;
    }
    currentUser = u;
    initStack();
    printf("Welcome %s!\n", u->name);
    userMenu();
}

int main() {
    initQueue();
    int choice;
    printf("=== Banking System ===\n");
    while(1) {
        printf("\n1.Create Account  2.Login  3.Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        
        if(choice == 1)
            createAccount();
        else if(choice == 2)
            login();
        else if(choice == 3) {
            printf("Goodbye!\n");
            break;
        }
        else
            printf("Invalid choice\n");
    }
    return 0;
}