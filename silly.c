#include <math.h>
#include <float.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

size_t f(float x, int n)
{
    bool is_negative = x < 0;
    x = fabsf(x);

    float pow10 = 1;
    float factor = n < 0 ? 0.1 : 10;

    size_t decs = 0;
    size_t zeroes = 0;
    for (int i = 0; decs < FLT_DIG and i < abs(n); i++) {
        float next_pow = pow10 * factor;
        float mod = fmodf(x * next_pow, 10);

        if (mod == 0)
            break;
        
        pow10 = next_pow;
        if (decs != 0 or (int)mod != 0)
            decs += 1;
        else
            zeroes += 1;
    }
    decs += n * (n < 0);

    uint_fast64_t x_integ = roundf(x * pow10);

    size_t ints = 0;
    for (int y = x_integ / pow10; y > 0; y /= 10)
        ints += 1;

    size_t len = (is_negative) + (ints == 0) + ints + (decs != 0) + zeroes + decs;
    char buf[len + 1];
    buf[len] = 0;

    for (int i = 0; i < decs; i++) {
        buf[--len] = x_integ % 10 + 48;
        x_integ /= 10;
    }

    for (int i = 0; i < zeroes; i++) {
        buf[--len] = '0';
    }

    if (decs != 0)
        buf[--len] = '.';

    if (n < 0) for (int i = 0; i < -n; i++) {
        buf[--len] = '0';
    }
    
    if (ints == 0) {
        buf[--len] = '0';
    } else for (int i = 0; i < ints; i++) {
        buf[--len] = x_integ % 10 + 48;
        x_integ /= 10;
    }

    if (is_negative)
        buf[--len] = '-';

    printf("%s\n", buf);
    printf("\t%f : decs:%llu, ints:%u, zeroes:%u\n", pow10, decs, ints, zeroes);
}

int main()
{
    f(39, 3);     // "39"
    f(39.39, 9);  // "39.39"
    f(0.3939, 3); // "0.394"
    f(39e-9, 9);  // "0.000000039"

    f(3939, -2);  // "3900"

    f(39.99, 0); // 39.99  expected: 40
    f(39.99, 1); // 39.    expected: 40
    f(39.99, 2); // 39.    expected: 39.99

    f(-39.3939, 3); // -39.4  expected: -39.394

    return 0;
}