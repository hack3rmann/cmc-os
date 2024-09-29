#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static double const MIN_PERCENTAGE = -100.0;
static double const MAX_PERCENTAGE = 100.0;
static double const MIN_EXCHANGE_RATE = 0.0;
static double const MAX_EXCHANGE_RATE = 10'000.0;

enum
{
    MIN_N_ARGUMENTS = 2,
};

double
parse_double(char *src)
{
    char *str_end = nullptr;
    errno = 0;

    auto const value = strtod(src, &str_end);

    if ('\0' != *str_end || 0 != errno || src == str_end) {
        fprintf(stderr, "invalid number %s\n", src);
        exit(EXIT_FAILURE);
    }

    return value;
}

bool
has_valid_precision(double value)
{
    return value == round(MAX_EXCHANGE_RATE * value) / MAX_EXCHANGE_RATE;
}

int
main(int argc, char *argv[])
{
    if (argc < MIN_N_ARGUMENTS) {
        fprintf(stderr, "program should have at least 1 number argument\n");
        exit(EXIT_FAILURE);
    }

    auto const initial_rate = parse_double(argv[1]);

    if (initial_rate <= MIN_EXCHANGE_RATE || initial_rate > MAX_EXCHANGE_RATE) {
        fprintf(stderr, "initial exchange rate should be positive and less than %lf\n", MAX_EXCHANGE_RATE);
        exit(EXIT_FAILURE);
    }

    if (!has_valid_precision(initial_rate)) {
        fprintf(stderr, "initial exchange rate is too precise, should have 4 digits after decimal point\n");
        exit(EXIT_FAILURE);
    }

    static double const EXCHANGE_RATE_MULTIPLIER = MAX_EXCHANGE_RATE;
    double exchange_rate = EXCHANGE_RATE_MULTIPLIER * initial_rate;

    for (size_t i = 2; i < (size_t) argc; ++i) {
        auto const percentage = parse_double(argv[i]);

        if (i != 1 && (percentage <= MIN_PERCENTAGE || percentage >= MAX_PERCENTAGE)) {
            fprintf(stderr, "exchange rate change percentage should be greater than %lf and less than %lf\n",
                    MIN_PERCENTAGE, MAX_PERCENTAGE);
            exit(EXIT_FAILURE);
        }

        double const multiplier = 1.0 + 0.01 * percentage;

        exchange_rate = round(exchange_rate * multiplier);

        if (exchange_rate <= EXCHANGE_RATE_MULTIPLIER * MIN_EXCHANGE_RATE ||
            exchange_rate > EXCHANGE_RATE_MULTIPLIER * MAX_EXCHANGE_RATE) {

            fprintf(stderr, "calculation error: exchange rate should always be less than %lf\n", MAX_EXCHANGE_RATE);
            exit(EXIT_FAILURE);
        }
    }

    double const result_rate = exchange_rate / EXCHANGE_RATE_MULTIPLIER;

    printf("%.4lf\n", result_rate);
}
