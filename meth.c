#include "stdio.h"
#define true 1
#define false 0
#define string char*

void read_file(string filename) {
    FILE *fptr;
    fptr = fopen(filename, "r");

    char c;
    while (c != -1) {
        c = fgetc(fptr);
        printf("%c", c);
    }

}

int main() {
    printf("hello \n");
    read_file("./expr.meth");
    return 0;
}
