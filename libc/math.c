#include "libc.h"

int abs(int x) {
    return x < 0 ? -x : x;
}

double pow(double base, double exp) {
    double result = 1.0;
    for (int i = 0; i < (int)exp; i++) {
        result *= base;
    }
    return result;
}

double sqrt(double x) {
    double guess = x / 2.0;
    double epsilon = 0.00001;
    while ((guess * guess - x) > epsilon || (x - guess * guess) > epsilon) {
        guess = (guess + x / guess) / 2.0;
    }
    return guess;
}
