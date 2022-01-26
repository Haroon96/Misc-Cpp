#include <iostream>

int sieve(int n) {
    // initialize all numbers as prime
    bool *is_prime = new bool[n + 1];
    for (int i = 0; i <= n; ++i) is_prime[i] = true;

    // start from first prime
    int p = 2;
    int count = 0;

    // run sieve algo for all values until n
    while (p <= n) {

        // mark multiples of p as not prime
        for (int i = p * 2; i <= n; i += p) {
            is_prime[i] = false;
        }

        // find next prime number
        for (p += 1; !is_prime[p] && p <= n; p++);

        // increment count of primes
        count += 1;
    }

    // clean up and return
    delete[]is_prime;
    return count;
}

int main(int argc, char *argv[]) {

    // check if user specified argument
    if (argc < 2) {
        std::cerr << "Not enough arguments!" << std::endl;
        return -1;
    }

    // convert arg to int
    int n = std::stoi(argv[1]);

    // run sieve algo
    std::cout << sieve(n) << std::endl;

    return 0;
}
