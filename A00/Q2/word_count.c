#include <stdio.h>
#include <ctype.h>

int countWordFromFile(char* argv[]);

int main(int argc, char* argv[]) {
    printf("%d %s\n", countWordFromFile(argv), argv[1]);
    return 0;
}

int countWordFromFile(char* argv[]) {
    FILE* fpt = fopen(argv[1], "r");
    int c;
    int prev = ' ';
    int wordCount = 0;

    while((c = fgetc(fpt)) != EOF) {
        if(!isspace(prev) && isspace(c)) {
            wordCount++;
        }
        prev = c;
    }

    if(!isspace(prev)) {
        wordCount++;
    }

    fclose(fpt);
    return wordCount;
}