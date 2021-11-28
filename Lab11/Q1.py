import random

random.seed(5)


def millerRabinPrimalityTest(d, n):
    """Performs the miller-rabin primality test
    :param d: int
    :param n: int
    :return: bool
    :returns If the number n is probably prime or not
    """

    # random number between [2..n-2] avoiding the case of n > 4
    a = 2 + random.randint(1, n - 4)

    # Compute a^d % n
    x = pow(a, d, n)
    if x == 1 or x == n - 1:
        return True

    # x is squared and modularized by n until
    # d wont reach n-1
    # (x^2) % n is not 1
    # (x^2) % n is not n-1
    while d != n - 1:
        x = pow(x, 2, n)
        d *= 2

        if x == 1:
            return False
        if x == n - 1:
            return True

    return False


def check_primality(n, rounds):
    """ Returns false if n is composite else true if n is probably prime
    :param n: int
    :param rounds: int (More number of rounds indicate more accuracy)
    :return: bool
    """

    # Avoiding base cases
    if n <= 1 or n % 2 == 0:
        return False
    if n <= 3:
        return True

    # n = 2^r * k + 1
    # Calculating r and k
    k = 0
    r = n - 1
    while r % 2 == 0:
        r //= 2
        k += 1

    # Loop for the appropriate rounds
    for i in range(rounds):
        if millerRabinPrimalityTest(r, n) == False:
            return False

    return True


def generatePrime(key_length=512, rounds=500):
    """Generates large prime number
    :param key_length: int
    :param rounds: int
    :return: int
    """
    prime_num = 1
    while not check_primality(prime_num, rounds):
        # Generating a random number of key_length bits
        prime_num = random.getrandbits(key_length)
        # Ensuring that the prime number is odd and there are no leading zero bits
        prime_num |= (1 << key_length - 1) | 1
    return prime_num


def gcd(a, b):
    """Calculates GCD of two numbers
    :param a: int
    :param b: int
    :return: int
    :returns GCD of a and b
    """
    temp = 0
    while True:
        temp = a % b
        if temp == 0:
            return b
        a = b
        b = temp


def calculate_coprime(x):
    """Calculates the first co-prime
    :param x: int
    :return: int
    :returns Co-prime of x
    """
    e = 2
    while e < x:
        if (gcd(e, x) == 1):
            break
        else:
            e += 1
    return e


def calculate_modular_mul_inv(e, phi):
    """ Calculate the modular multiplicative inverse
    :param e: int
    :param phi: int
    :return: int
    """
    q = 1

    while ((((q * phi) + 1) % e != 0)):
        q += 1

    d = ((q * phi) + 1) // e
    return d


def create_keys(public_key_file, private_key_file):
    """ Create public and private rsa keys
    :param public_key_file: str
    :param private_key_file: str
    :return: None
    """

    # Generate Primes
    prime_a = generatePrime()
    prime_b = generatePrime()

    # Calculating modulus for the keys
    n = prime_a * prime_b

    # Calculating the totient function
    totient_fn = (prime_a - 1) * (prime_b - 1)

    # Calculating coprime
    d = calculate_coprime(totient_fn)

    # Calulating modular multiplicative inverse
    e = calculate_modular_mul_inv(d, totient_fn)

    public_key_str = str(e) + "," + str(n)
    private_key_str = str(d) + "," + str(n)

    f = open(public_key_file, "w")
    f.write(public_key_str)
    f.close()

    f = open(private_key_file, "w")
    f.write(private_key_str)
    f.close()


def main():
    # Creating and saving the keys
    create_keys("A.pub", "A.pri")
    print("Generated public and private keys for user A and stored it in - A.pub, A.pri respectively")

    create_keys("B.pub", "B.pri")
    print("Generated public and private keys for user B and stored it in - B.pub, B.pri respectively")


if __name__ == "__main__":
    main()
