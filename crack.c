#include<stdio.h>
#include<crypt.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include<pthread.h>

//GRADING NOTE:
// All test cases passed, good parallel speedup shown.
// Second extra credit completed.

// Score: 102/100

void newPassword(char* str);
char* hash_and_check(char* password, char* salt, char* target, struct crypt_data* data);
char* encrypt(char* password, char* salt);
void* thread_entry(void* args);

struct threadArgs {
    int threads;
    int keySize;
    char* target;
    char* salt;
    char start;
    char end;
};

int main(int argc, char* argv[]) {
    struct threadArgs arguments;

    if (argc == 1) {
        printf("Usage: crack <threads> <keysize> <target> \n");
        exit(0);
    }
    if (argc != 4) {
        printf("Incorrect useage. Please type './crack' to find the correct useage statement. \n");
        exit(0);
    }

    arguments.threads = atoi(argv[1]);
    arguments.keySize = atoi(argv[2]);
    arguments.target = argv[3];

    if (arguments.threads <= 0 || arguments.threads > 26) {
        printf("<threads> must be between 1 and 26 \n");
        exit(-1);
    }

    if (arguments.keySize <= 0 || arguments.keySize > 8) {
        printf("<keysize> must be between 0 and 8 \n");
        exit(-1);
    }

    char salt[3];
    strncpy(salt, &arguments.target[0], 2);
    salt[2] = '\0';
    arguments.salt = salt;

    //set up threads
    struct threadArgs myThreads[arguments.threads];
    pthread_t threads[arguments.threads];

    int totalChars = 26;
    int numThreads = arguments.threads;
    int currentChar = 96;

    for (int i = 0; i < arguments.threads; i++) {
        if (numThreads > 0) {
            int intDiv = totalChars / numThreads;
            totalChars -= intDiv;
            myThreads[i].start = currentChar + 1;
            myThreads[i].end = myThreads[i].start + intDiv - 1;
            myThreads[i].keySize = arguments.keySize;
            myThreads[i].target = arguments.target;
            myThreads[i].salt = arguments.salt;
        }
        if (myThreads[i].end > 122) {
            myThreads[i].end = 122;
        }
        pthread_create(&threads[i], NULL, thread_entry, &myThreads[i]);
        numThreads--;
        currentChar = myThreads[i].end;
    }

    for (int i = 0; i < arguments.threads; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}

void* thread_entry(void* args) {
    struct threadArgs* argptr = args;
    struct crypt_data myData;
    myData.initialized = 0;

    // test all keysizes
    for (int key = 0; key < argptr->keySize; key++) {
        //checks all possibilities 1-z of keysize
        char* password = (char*)malloc(sizeof(char)*(key + 1));
        memset(password, argptr->start, key + 1);
        password[key + 1] = '\0';

        //break if get to final password
        char* stringToTest = (char*)malloc(sizeof(char)*(key + 1));
        memset(stringToTest, 'z', key + 1);
        stringToTest[0] = argptr->end;
        stringToTest[key + 1] = '\0';

        while(1) {
            if(hash_and_check(password, argptr->salt, argptr->target, &myData)) {
                break;
            }
            // test combinations
            newPassword(password);
            if (!strcmp(password, stringToTest)) {
                break;
            }
        }
    }
}

char* hash_and_check(char* password, char* salt, char* target, struct crypt_data* data){
    char* res = crypt_r(password, salt, data);
    if (strcmp(target, res) == 0) {
        printf("The password is: %s\n", password);
        return password;
    }
    else {
        return NULL;
    }
}

void newPassword(char *str){
    int index, carry;
    for(index = strlen(str)-1;index>=0;--index){
        if(str[index] == 'z'){
            carry = 1;
            str[index] = 'a';
        } else {
            carry = 0;
            str[index] += 1;
        }
        if(carry == 0)break;
    }
}
