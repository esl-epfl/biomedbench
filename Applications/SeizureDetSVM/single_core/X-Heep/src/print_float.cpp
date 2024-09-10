// Author: Francesco Poluzzi         //
#include "print_float.hpp"

extern "C" {
    #include <stdio.h>
    #include <math.h>
}

void print_float_cpp(float number, int decimal_places) {
    // Handle negative numbers
    if (number < 0) {
        printf("-");
        number = -number;
    }

    // Get the integer part of the number
    int integer_part = static_cast<int>(number);
    // Get the fractional part of the number
    float fractional_part = number - integer_part;


    // Print the integer part
    printf("%d.", integer_part);

    // Print the fractional part
    for (int i = 0; i < decimal_places; i++) {
        fractional_part *= 10;
        int digit = static_cast<int>(fractional_part);
        printf("%d", digit);
        fractional_part -= digit;
    }
}