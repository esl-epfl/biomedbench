#ifndef PRINT_FLOAT_H
#define PRINT_FLOAT_H

#include <math.h>
#include <stdio.h>

void print_float(float number, int decimal_places) {
    // Handle negative numbers
    if (number < 0) {
        printf("-");
        number = -number;
    }

    // Get the integer part of the number
    int integer_part = (int)number;
    // Get the fractional part of the number
    float fractional_part = number - integer_part;

    // Print the integer part
    printf("%d.", integer_part);

    // Print the fractional part
    for (int i = 0; i < decimal_places; i++) {
        fractional_part *= 10;
        int digit = (int)fractional_part;
        printf("%d", digit);
        fractional_part -= digit;
    }
}

#endif // PRINT_FLOAT_H