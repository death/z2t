/*
 * flow-gen-message - Generate a message for testing z2t flow.
 */
#include <stdio.h>

enum {
    WIDTH = 640,
    HEIGHT = 480
};

struct complex
{
    float real;
    float imag;
};

struct complex *complex_set(struct complex *c, float real, float imag);
struct complex *complex_add(struct complex *a, struct complex *b);
struct complex *complex_mul(struct complex *a, struct complex *b);
float complex_magsq(struct complex *c);
int min(int a, int b);

int main(void)
{
    int y;
    struct complex c;

    printf("P5\n%d\n%d\n%d\n", WIDTH, HEIGHT, 255);

    complex_set(&c, -0.1, 0.8);

    for (y = 0; y < HEIGHT; y++) {
        int x;

        for (x = 0; x < WIDTH; x++) {
            int i;
            struct complex n;

            complex_set(&n,
                        -2.0 + (x / (WIDTH / 4.0)),
                        +2.0 - (y / (HEIGHT / 4.0)));

            for (i = 0; i < 255 && complex_magsq(&n) <= 4.0; i++) {
                complex_add(complex_mul(&n, &n), &c);
            }

            fputc(min(255, 6 * (i == 255 ? 0 : i)), stdout);
        }
    }

    return 0;
}

struct complex *complex_set(struct complex *c, float real, float imag)
{
    c->real = real;
    c->imag = imag;
    return c;
}

struct complex *complex_add(struct complex *a, struct complex *b)
{
    return complex_set(a,
                       a->real + b->real,
                       a->imag + b->imag);
}

struct complex *complex_mul(struct complex *a, struct complex *b)
{
    return complex_set(a,
                       (a->real * b->real) - (a->imag * b->imag),
                       (a->imag * b->real) + (a->real * b->imag));
}

float complex_magsq(struct complex *c)
{
    return c->real * c->real + c->imag * c->imag;
}

int min(int a, int b)
{
    return a < b ? a : b;
}
