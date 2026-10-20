#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SLOTS 10
#define REG_LEN 20
#define NAME_LEN 50

typedef struct Vehicle {
    char number[REG_LEN];
    char owner[NAME_LEN];
    char type[16];
    int slot;
    int parked;
    struct Vehicle *next;
} Vehicle;

typedef struct WaitNode {
    char number[REG_LEN];
    char owner[NAME_LEN];
    char type[16];
    struct WaitNode *next;
} WaitNode;

typedef struct {
    int occupied;
    char number[REG_LEN];
} ParkingSlot;

ParkingSlot slots[MAX_SLOTS];
Vehicle *vehicleHead = NULL;
WaitNode *waitFront = NULL, *waitRear = NULL;

void line(void) {
    printf("------------------------------------------------------------\n");
}

void readText(const char *prompt, char *out, size_t size) {
    printf("%s", prompt);
    if (fgets(out, (int)size, stdin) == NULL) {
        out[0] = '\0';
        return;
    }
    size_t n = strlen(out);
    if (n && out[n - 1] == '\n') out[n - 1] = '\0';
    else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
}

int readInt(const char *prompt, int min, int max) {
    char buf[100], *end;
    long val;
    for (;;) {
        readText(prompt, buf, sizeof(buf));
        val = strtol(buf, &end, 10);
        while (isspace((unsigned char)*end)) end++;
        if (buf[0] && *end == '\0' && val >= min && val <= max)
            return (int)val;
        printf("Invalid input. Enter a number between %d and %d.\n", min, max);
    }
}

Vehicle *findVehicle(const char *number) {
    Vehicle *cur = vehicleHead;
    while (cur) {
        if (strcmp(cur->number, number) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}
int findFreeSlot(void) {
    int i;
    for (i = 0; i < MAX_SLOTS; i++)
        if (!slots[i].occupied) return i;
    return -1;
}

int isQueued(const char *number) {
    WaitNode *cur = waitFront;
    while (cur) {
        if (strcmp(cur->number, number) == 0) return 1;
        cur = cur->next;
    }
    return 0;
}

void enqueueWait(const char *number, const char *owner, const char *type) {
    WaitNode *node = (WaitNode *)malloc(sizeof(WaitNode));
    if (!node) {
        printf("Memory allocation failed; vehicle not added to waiting queue.\n");
        return;
    }
    snprintf(node->number, sizeof(node->number), "%s", number);
    snprintf(node->owner, sizeof(node->owner), "%s", owner);
    snprintf(node->type, sizeof(node->type), "%s", type);
    node->next = NULL;
    if (!waitRear) waitFront = waitRear = node;
    else {
        waitRear->next = node;
        waitRear = node;
    }
    printf("Parking full. %s added to the first-come, first-served waiting queue.\n", number);
}

void assignVehicle(const char *number, const char *owner,
                          const char *type) {
    int idx = findFreeSlot();
    if (idx < 0) {
        enqueueWait(number, owner, type);
        return;
    }
    Vehicle *v = findVehicle(number);
    if (!v) {
        v = (Vehicle *)malloc(sizeof(Vehicle));
        if (!v) {
            printf("Memory allocation failed. Vehicle not registered.\n");
            return;
        }
        memset(v, 0, sizeof(*v));
        snprintf(v->number, sizeof(v->number), "%s", number);
        v->next = vehicleHead;
        vehicleHead = v;
    }
    snprintf(v->owner, sizeof(v->owner), "%s", owner);
    snprintf(v->type, sizeof(v->type), "%s", type);
    v->slot = idx + 1;
    v->parked = 1;
    slots[idx].occupied = 1;
    snprintf(slots[idx].number, sizeof(slots[idx].number), "%s", number);
    printf("Vehicle %s parked successfully. Assigned slot: %d\n", number, idx + 1);
}

void vehicleEntry(void) {
    char number[REG_LEN], owner[NAME_LEN], type[16];
    printf("\nVEHICLE ENTRY\n");
    line();
    readText("Registration number: ", number, sizeof(number));
    if (!number[0]) { printf("Registration number cannot be empty.\n"); return; }
    Vehicle *existing = findVehicle(number);
    if ((existing && existing->parked) || isQueued(number)) {
        printf("This vehicle is already parked or waiting.\n");
        return;
    }
    readText("Owner name: ", owner, sizeof(owner));
    if (!owner[0]) { printf("Owner name cannot be empty.\n"); return; }
    printf("Vehicle type: 1. Two-wheeler  2. Four-wheeler\n");
    int choice = readInt("Select type: ", 1, 2);
    snprintf(type, sizeof(type), "%s", choice == 1 ? "Two-wheeler" : "Four-wheeler");
    assignVehicle(number, owner, type);
}

void removeFromQueueAndAssign(void) {
    if (!waitFront) return;
    int idx = findFreeSlot();
    if (idx < 0) return;
    WaitNode *node = waitFront;
    waitFront = node->next;
    if (!waitFront) waitRear = NULL;
    assignVehicle(node->number, node->owner, node->type);
    free(node);
}

void vehicleExit(void) {
    char number[REG_LEN];
    int fee = 30;
    printf("\nVEHICLE EXIT\n");
    line();
    readText("Registration number: ", number, sizeof(number));
    Vehicle *v = findVehicle(number);
    if (!v || !v->parked) {
        printf("Parked vehicle not found.\n");
        return;
    }
    printf("Vehicle number : %s\n", v->number);
    printf("Parking slot   : %d\n", v->slot);
    printf("Parking fee    : Rs. %d (fixed fee)\n", fee);
    slots[v->slot - 1].occupied = 0;
    slots[v->slot - 1].number[0] = '\0';
    v->parked = 0;
    v->slot = 0;
    printf("Slot released successfully.\n");
    removeFromQueueAndAssign();
}

void searchVehicle(void) {
    char number[REG_LEN];
    readText("\nEnter registration number to search: ", number, sizeof(number));
    Vehicle *v = findVehicle(number);
    if (!v) { printf("Vehicle record not found.\n"); return; }
    printf("\nVehicle details\n");
    line();
    printf("Registration : %s\nOwner        : %s\nType         : %s\n",
           v->number, v->owner, v->type);
    if (v->parked) printf("Status       : PARKED\nSlot         : %d\n", v->slot);
    else printf("Status       : NOT CURRENTLY PARKED\n");
}

void displayParking(void) {
    int occupied = 0;
    int i;
    printf("\nPARKING SLOT STATUS (%d SLOTS)\n", MAX_SLOTS);
    line();
    printf("%-10s %-14s %-18s\n", "Slot", "Status", "Vehicle");
    line();
    for (i = 0; i < MAX_SLOTS; i++) {
        printf("%-10d %-14s %-18s\n", i + 1,
               slots[i].occupied ? "OCCUPIED" : "AVAILABLE",
               slots[i].occupied ? slots[i].number : "-");
        occupied += slots[i].occupied;
    }
    printf("Occupied: %d | Available: %d\n", occupied, MAX_SLOTS - occupied);
}

void displayVehicles(void) {
    Vehicle *v = vehicleHead;
    printf("\nVEHICLE RECORDS\n");
    line();
    printf("%-18s %-20s %-15s %-10s %-6s\n",
           "Registration", "Owner", "Type", "Status", "Slot");
    line();
    if (!v) { printf("No vehicle records available.\n"); return; }
    while (v) {
        printf("%-18s %-20.20s %-15s %-10s %-6d\n",
               v->number, v->owner, v->type,
               v->parked ? "PARKED" : "EXITED", v->parked ? v->slot : 0);
        v = v->next;
    }
}

void displayWaitingQueue(void) {
    WaitNode *cur = waitFront;
    printf("\nWAITING QUEUE (FIRST COME, FIRST SERVED)\n");
    line();
    if (!cur) { printf("No vehicles waiting.\n"); return; }
    while (cur) {
        printf("%s (%s) -> ", cur->number, cur->type);
        cur = cur->next;
    }
    printf("END\n");
}

void sortVehicles(void) {
    int n = 0;
    int i;
    int j;
    int choice;
    Vehicle *v;
    Vehicle **arr;

    for (v = vehicleHead; v != NULL; v = v->next) {
        n++;
    }

    if (n < 2) {
        printf("Not enough vehicle records to sort.\n");
        return;
    }

    arr = (Vehicle **)malloc((size_t)n * sizeof(Vehicle *));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    i = 0;
    for (v = vehicleHead; v != NULL; v = v->next) {
        arr[i++] = v;
    }

    printf("1. Registration number (A-Z)\n");
    printf("2. Parking status (parked first)\n");
    choice = readInt("Sort by: ", 1, 2);

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            int swap = 0;

            if (choice == 1 &&
                strcmp(arr[j]->number, arr[j + 1]->number) > 0) {
                swap = 1;
            }

            if (choice == 2 &&
                arr[j]->parked < arr[j + 1]->parked) {
                swap = 1;
            }

            if (swap) {
                Vehicle *tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }

    printf("\nSORTED VEHICLE RECORDS\n");
    line();
    printf("%-18s %-20s %-15s %-10s %-6s\n",
           "Registration", "Owner", "Type", "Status", "Slot");
    line();

    for (i = 0; i < n; i++) {
        printf("%-18s %-20.20s %-15s %-10s %-6d\n",
               arr[i]->number, arr[i]->owner, arr[i]->type,
               arr[i]->parked ? "PARKED" : "EXITED",
               arr[i]->parked ? arr[i]->slot : 0);
    }

    free(arr);
}

void cleanup(void) {
    while (vehicleHead) {
        Vehicle *next = vehicleHead->next;
        free(vehicleHead);
        vehicleHead = next;
    }
    while (waitFront) {
        WaitNode *next = waitFront->next;
        free(waitFront);
        waitFront = next;
    }
}

int main(void) {
    int choice;
   do{
        printf("\n");
        line();
        printf("          PARKING MANAGEMENT SYSTEM\n");
        printf("             DSA MINI PROJECT\n");
        line();
        printf("1. Vehicle Entry / Park\n");
        printf("2. Vehicle Exit / Calculate Fee\n");
        printf("3. Search Vehicle\n");
        printf("4. Display Parking Slots\n");
        printf("5. Display All Vehicle Records\n");
        printf("6. Display Waiting Queue\n");
        printf("7. Sort Vehicle Records\n");
        printf("8. Exit\n");
        choice = readInt("Enter your choice: ", 1, 8);
        switch (choice) {
            case 1:
                vehicleEntry();
                break;
            case 2:
                vehicleExit();
                break;
            case 3:
                 searchVehicle();
                  break;
            case 4:
                displayParking();
                 break;
            case 5:
                displayVehicles();
                 break;
            case 6:
                 displayWaitingQueue();
                  break;
            case 7:
                sortVehicles();
                break;
            case 8:
                cleanup();
                printf("Thank you for using Parking Management System.\n");
                return 0;
        }
    }while(choice<9);
}
