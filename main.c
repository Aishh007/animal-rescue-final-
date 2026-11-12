#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX 50
#define NAME_LEN 30
#define TYPE_LEN 20
#define LOC_LEN 40

/* Status codes */
#define REPORTED 0
#define RESCUED  1
#define TREATED  2   /* treated and ready for adoption */
#define ADOPTED  3

/* Condition codes: higher number = more urgent */
/* 1 = Healthy, 2 = Sick, 3 = Injured, 4 = Critical */

typedef struct Animal{
    int id;
    char type[TYPE_LEN];
    char location[LOC_LEN];
    int condition;
    int status;
    char rescuer[NAME_LEN];
    char adopter[NAME_LEN];
}Animal;

/* Activity log: a linked list used like a stack (newest on top) */
typedef struct Log{
    char text[80];
    struct Log *next;
}Log;

/* Max-heap that stores animal indices (most urgent on top) */
typedef struct Heap{
    int index[MAX];
    int size;
}Heap;

Animal animals[MAX];
int n=0;            /* number of animals */
int nextId=1;       /* id given to the next reported animal */
Log *logTop=NULL;
Heap urgent;

const char *conditionName[]={"","Healthy","Sick","Injured","Critical"};
const char *statusName[]={"Reported","Rescued","Ready for adoption","Adopted"};


/* ---------- Input helpers ---------- */

static void readLine(const char *prompt, char *buf, int size){
    printf("%s",prompt);
    if(fgets(buf,size,stdin)==NULL){
        printf("\nInput closed. Exiting.\n");
        exit(0);
    }
    if(strchr(buf,'\n')==NULL){
        int ch;
        while((ch=getchar())!='\n' && ch!=EOF);
    }
    buf[strcspn(buf,"\n")]='\0';
}

static int readInt(const char *prompt){
    char b[64];
    readLine(prompt,b,sizeof(b));
    return atoi(b);
}

/* returns the position of an animal with this id, or -1 */
static int findById(int id){
    for(int i=0;i<n;i++){
        if(animals[i].id==id){
            return i;
        }
    }
    return -1;
}


/* ---------- Activity log (linked list / stack) ---------- */

static void addLog(const char *text){
    Log *l=calloc(1,sizeof(Log));
    if(l==NULL){
        return;
    }
    strncpy(l->text,text,sizeof(l->text)-1);
    l->next=logTop;
    logTop=l;
}

static void showLog(void){
    printf("\n--- Recent activity (newest first) ---\n");
    if(logTop==NULL){
        printf("No activity yet.\n");
        return;
    }
    Log *temp=logTop;
    int count=0;
    while(temp!=NULL && count<10){
        printf(" - %s\n",temp->text);
        temp=temp->next;
        count++;
    }
}


/* ---------- Heap (priority for rescue) ---------- */

/* returns 1 if animal a is more urgent than animal b */
static int moreUrgent(int a,int b){
    if(animals[a].condition!=animals[b].condition){
        return animals[a].condition>animals[b].condition;
    }
    return animals[a].id<animals[b].id;   /* same condition: older report first */
}

static void heapInsert(Heap *h,int animalIndex){
    int i=h->size;
    h->index[i]=animalIndex;
    h->size++;
    while(i>0){
        int parent=(i-1)/2;
        if(!moreUrgent(h->index[i],h->index[parent])){
            break;
        }
        int temp=h->index[parent];
        h->index[parent]=h->index[i];
        h->index[i]=temp;
        i=parent;
    }
}

static int heapExtract(Heap *h){
    int result=h->index[0];
    h->size--;
    h->index[0]=h->index[h->size];
    int i=0;
    while(1){
        int left=2*i+1;
        int right=2*i+2;
        int best=i;
        if(left<h->size && moreUrgent(h->index[left],h->index[best])){
            best=left;
        }
        if(right<h->size && moreUrgent(h->index[right],h->index[best])){
            best=right;
        }
        if(best==i){
            break;
        }
        int temp=h->index[i];
        h->index[i]=h->index[best];
        h->index[best]=temp;
        i=best;
    }
    return result;
}

static void buildHeap(void){
    urgent.size=0;
    for(int i=0;i<n;i++){
        if(animals[i].status==REPORTED){
            heapInsert(&urgent,i);
        }
    }
}


/* ---------- Main features ---------- */

static void printAnimal(int i){
    printf("%-4d %-10s %-16s %-9s %-19s %-12s %-12s\n",
           animals[i].id,animals[i].type,animals[i].location,
           conditionName[animals[i].condition],statusName[animals[i].status],
           animals[i].rescuer,animals[i].adopter);
}

static void printHeader(void){
    printf("%-4s %-10s %-16s %-9s %-19s %-12s %-12s\n",
           "ID","Type","Location","Condition","Status","Rescuer","Adopter");
    printf("-------------------------------------------------------------------------------------\n");
}

static void reportAnimal(void){
    if(n>=MAX){
        printf("Sorry, the system is full (maximum %d animals).\n",MAX);
        return;
    }
    char type[TYPE_LEN];
    char location[LOC_LEN];
    readLine("Animal type (e.g. Dog, Cat, Cow): ",type,sizeof(type));
    if(type[0]=='\0'){
        printf("The type can't be empty. Please try again.\n");
        return;
    }
    readLine("Where was it seen? (area/street): ",location,sizeof(location));
    if(location[0]=='\0'){
        printf("The location can't be empty. Please try again.\n");
        return;
    }
    printf("What is its condition?\n");
    printf(" 1. Healthy\n 2. Sick\n 3. Injured\n 4. Critical\n");
    int cond=readInt("Enter number (1-4): ");
    if(cond<1 || cond>4){
        printf("That number isn't in the list.\n");
        return;
    }
    animals[n].id=nextId;
    strncpy(animals[n].type,type,TYPE_LEN-1);
    animals[n].type[TYPE_LEN-1]='\0';
    strncpy(animals[n].location,location,LOC_LEN-1);
    animals[n].location[LOC_LEN-1]='\0';
    animals[n].condition=cond;
    animals[n].status=REPORTED;
    strcpy(animals[n].rescuer,"-");
    strcpy(animals[n].adopter,"-");
    char msg[80];
    snprintf(msg,sizeof(msg),"Reported %s #%d at %.20s",type,nextId,location);
    addLog(msg);
    printf("Reported! %s has been given ID %d.\n",type,nextId);
    n++;
    nextId++;
}

static void showAnimals(void){
    printf("\n--- All animals (%d) ---\n",n);
    if(n==0){
        printf("No animals yet. Report one with option 1.\n");
        return;
    }
    printHeader();
    for(int i=0;i<n;i++){
        printAnimal(i);
    }
}

static void toLower(char *s){
    for(int i=0;s[i]!='\0';i++){
        s[i]=(char)tolower((unsigned char)s[i]);
    }
}

static void searchByLocation(void){
    char key[LOC_LEN];
    readLine("Enter area to search: ",key,sizeof(key));
    if(key[0]=='\0'){
        printf("Nothing to search for.\n");
        return;
    }
    toLower(key);
    int found=0;
    for(int i=0;i<n;i++){
        char loc[LOC_LEN];
        strcpy(loc,animals[i].location);
        toLower(loc);
        if(strstr(loc,key)!=NULL){
            if(found==0){
                printf("\n");
                printHeader();
            }
            printAnimal(i);
            found++;
        }
    }
    if(found==0){
        printf("No animals found in that area.\n");
    }
}

static void rescueNext(void){
    buildHeap();
    if(urgent.size==0){
        printf("\nNo animals are waiting to be rescued.\n");
        return;
    }
    int i=heapExtract(&urgent);
    printf("\nMost urgent case: %s #%d at %s (%s)\n",
           animals[i].type,animals[i].id,animals[i].location,
           conditionName[animals[i].condition]);
    char name[NAME_LEN];
    readLine("Rescuer's name: ",name,sizeof(name));
    if(name[0]=='\0'){
        printf("The name can't be empty. Rescue not assigned.\n");
        return;
    }
    strncpy(animals[i].rescuer,name,NAME_LEN-1);
    animals[i].rescuer[NAME_LEN-1]='\0';
    animals[i].status=RESCUED;
    char msg[80];
    snprintf(msg,sizeof(msg),"%.20s rescued %s #%d",name,animals[i].type,animals[i].id);
    addLog(msg);
    printf("Done! %s will rescue %s #%d.\n",name,animals[i].type,animals[i].id);
}

static void markTreated(void){
    int found=0;
    for(int i=0;i<n;i++){
        if(animals[i].status==RESCUED){
            if(found==0){
                printf("\n--- Rescued animals waiting for treatment ---\n");
                printHeader();
            }
            printAnimal(i);
            found++;
        }
    }
    if(found==0){
        printf("\nNo rescued animals are waiting for treatment.\n");
        return;
    }
    int id=readInt("Enter the ID of the treated animal: ");
    int i=findById(id);
    if(i<0 || animals[i].status!=RESCUED){
        printf("That ID is not in the list above.\n");
        return;
    }
    animals[i].status=TREATED;
    animals[i].condition=1;   /* healthy after treatment */
    char msg[80];
    snprintf(msg,sizeof(msg),"%s #%d treated, ready for adoption",animals[i].type,id);
    addLog(msg);
    printf("%s #%d is now healthy and ready for adoption!\n",animals[i].type,id);
}

static void adoptAnimal(void){
    int found=0;
    for(int i=0;i<n;i++){
        if(animals[i].status==TREATED){
            if(found==0){
                printf("\n--- Animals ready for adoption ---\n");
                printHeader();
            }
            printAnimal(i);
            found++;
        }
    }
    if(found==0){
        printf("\nNo animals are ready for adoption yet.\n");
        return;
    }
    int id=readInt("Enter the ID of the animal to adopt: ");
    int i=findById(id);
    if(i<0 || animals[i].status!=TREATED){
        printf("That ID is not in the list above.\n");
        return;
    }
    char name[NAME_LEN];
    readLine("Adopter's name: ",name,sizeof(name));
    if(name[0]=='\0'){
        printf("The name can't be empty. Adoption cancelled.\n");
        return;
    }
    strncpy(animals[i].adopter,name,NAME_LEN-1);
    animals[i].adopter[NAME_LEN-1]='\0';
    animals[i].status=ADOPTED;
    char msg[80];
    snprintf(msg,sizeof(msg),"%s #%d adopted by %.20s",animals[i].type,id,name);
    addLog(msg);
    printf("Congratulations! %s #%d was adopted by %s.\n",animals[i].type,id,name);
}

static void undoLastReport(void){
    if(n==0){
        printf("There are no reports to undo.\n");
        return;
    }
    if(animals[n-1].status!=REPORTED){
        printf("The last animal has already been rescued, so it can't be undone.\n");
        return;
    }
    char msg[80];
    snprintf(msg,sizeof(msg),"Undid report of %s #%d",animals[n-1].type,animals[n-1].id);
    printf("Removed the last report (%s #%d).\n",animals[n-1].type,animals[n-1].id);
    n--;
    nextId--;
    addLog(msg);
}

static void showStats(void){
    int statusCount[4]={0};
    int condCount[5]={0};
    for(int i=0;i<n;i++){
        statusCount[animals[i].status]++;
        if(animals[i].status==REPORTED){
            condCount[animals[i].condition]++;
        }
    }
    printf("\n--- Statistics ---\n");
    printf("Total animals reported : %d\n",n);
    for(int s=0;s<4;s++){
        printf("%-20s: %d\n",statusName[s],statusCount[s]);
    }
    printf("\nStill waiting for rescue, by condition:\n");
    for(int c=4;c>=1;c--){
        printf("  %-10s: %d\n",conditionName[c],condCount[c]);
    }
}


/* ---------- Save and load ---------- */

static void saveData(void){
    FILE *fp=fopen("rescue.txt","w");
    if(fp==NULL){
        printf("Sorry, the data could not be saved.\n");
        return;
    }
    fprintf(fp,"%d\n",nextId);
    fprintf(fp,"%d\n",n);
    for(int i=0;i<n;i++){
        fprintf(fp,"%d\n",animals[i].id);
        fprintf(fp,"%s\n",animals[i].type);
        fprintf(fp,"%s\n",animals[i].location);
        fprintf(fp,"%d\n",animals[i].condition);
        fprintf(fp,"%d\n",animals[i].status);
        fprintf(fp,"%s\n",animals[i].rescuer);
        fprintf(fp,"%s\n",animals[i].adopter);
    }
    fclose(fp);
    printf("Data saved!\n");
}

/* reads one line from a file and removes the newline */
static void fileLine(FILE *fp,char *buf,int size){
    if(fgets(buf,size,fp)==NULL){
        buf[0]='\0';
        return;
    }
    buf[strcspn(buf,"\n")]='\0';
}

static void loadData(void){
    FILE *fp=fopen("rescue.txt","r");
    if(fp==NULL){
        printf("No saved data found. Starting fresh.\n");
        return;
    }
    char line[64];
    memset(animals,0,sizeof(animals));
    fileLine(fp,line,sizeof(line));
    nextId=atoi(line);
    fileLine(fp,line,sizeof(line));
    n=atoi(line);
    if(n>MAX){
        n=MAX;
    }
    if(n<0){
        n=0;
    }
    for(int i=0;i<n;i++){
        fileLine(fp,line,sizeof(line));
        animals[i].id=atoi(line);
        fileLine(fp,animals[i].type,TYPE_LEN);
        fileLine(fp,animals[i].location,LOC_LEN);
        fileLine(fp,line,sizeof(line));
        animals[i].condition=atoi(line);
        fileLine(fp,line,sizeof(line));
        animals[i].status=atoi(line);
        fileLine(fp,animals[i].rescuer,NAME_LEN);
        fileLine(fp,animals[i].adopter,NAME_LEN);
        /* basic safety checks on loaded values */
        if(animals[i].condition<1 || animals[i].condition>4){
            animals[i].condition=1;
        }
        if(animals[i].status<0 || animals[i].status>3){
            animals[i].status=REPORTED;
        }
    }
    fclose(fp);
    printf("Data loaded! %d animals.\n",n);
}


/* ---------- Menu ---------- */

static void menuLine(const char *text){
    printf("| %-40s |\n",text);
}

static void printMenu(void){
    char line[80];
    printf("\n+------------------------------------------+\n");
    menuLine("   PawRescue - Stray Animal Rescue");
    printf("+------------------------------------------+\n");
    snprintf(line,sizeof(line),"Animals in system: %d",n);
    menuLine(line);
    printf("+------------------------------------------+\n");
    menuLine("1.  Report a stray animal");
    menuLine("2.  See all animals");
    menuLine("3.  Search by location");
    menuLine("4.  Rescue next (most urgent first)");
    menuLine("5.  Mark animal as treated");
    menuLine("6.  Adopt an animal");
    menuLine("7.  Undo last report");
    menuLine("8.  See recent activity");
    menuLine("9.  See statistics");
    menuLine("10. Save data");
    menuLine("11. Load saved data");
    menuLine("12. Exit");
    printf("+------------------------------------------+\n");
}

int main(){
    urgent.size=0;
    loadData();
    int choice;

    while(1){
        printMenu();
        choice=readInt("Enter your choice (1-12): ");

        switch(choice){
            case 1:  reportAnimal();     break;
            case 2:  showAnimals();      break;
            case 3:  searchByLocation(); break;
            case 4:  rescueNext();       break;
            case 5:  markTreated();      break;
            case 6:  adoptAnimal();      break;
            case 7:  undoLastReport();   break;
            case 8:  showLog();          break;
            case 9:  showStats();        break;
            case 10: saveData();         break;
            case 11: loadData();         break;
            case 12:
                saveData();
                printf("PawRescue closed. Thank you for helping animals!\n");
                break;
            default:
                printf("Invalid option. Please enter a number from 1 to 12.\n");
        }

        if(choice==12){
            break;
        }
    }

    /* free the activity log */
    while(logTop!=NULL){
        Log *temp=logTop;
        logTop=logTop->next;
        free(temp);
    }
    return 0;
}