#include <stdio.h>
#include <string.h>

int main(void) {
    const char *correct_string = "Test";
    char input[128];

    printf("Enter string: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return 1;
    }

    input[strcspn(input, "\r\n")] = '\0';

    if (strcmp(input, correct_string) == 0) {
        printf("Correct!\n");
    } else {
        printf("wrong string!\n");
    }

    return 0;
}

