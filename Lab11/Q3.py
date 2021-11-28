def convertToString(v):
    """Converting integer -> binary -> string
    :param v: int
    :return: str
    """

    bin_str = "{0:08b}".format(v)  # convert integer into binary
    temp = ""
    actual_str = ""
    count = 0

    # Converting 8-bit chunks into corresponding character and appending into a string
    for i in range(len(bin_str) - 1, -1, -1):

        temp += bin_str[i]

        if (count + 1) % 8 == 0:
            temp = temp[::-1]
            num = int(temp, 2)
            actual_str += chr(num)
            temp = ""
            count = 0
            continue

        if i == 0:
            temp = temp[::-1]
            num = int(temp, 2)
            actual_str += chr(num)
            temp = ""

        count += 1
    return actual_str[::-1]


def decode(d, n, c):
    """ Decodes the message m
    :param d: int (d is part of the private key)
    :param n: int (n is part of the private key)
    :param c: int (c is the cipher text)
    :return: int (encoded message)
    """
    dec_message = pow(c, d, n)
    return dec_message


def read_file(filename):
    """Reading keys
    :param filename: str
    :return: (int, int)
    """
    f = open(filename, "r")
    e, n = list(map(int, f.read().split(",")))
    f.close()
    return e, n


def main():
    """Driver Function
    """

    # Reading keys
    e, n1 = read_file("A.pub")
    d, n2 = read_file("B.pri")
    public_B_e, public_B_n = read_file("B.pub")

    # Reading the encrypted text
    f = open("secret.txt", "r")
    enc = list(map(int, f.read().split(" ")))

    original_string = ""

    # Going through the chunks of ciphertext
    for st in enc:
        decoded_B_private_key = decode(d, n2, st) # Decode with B's private key
        decoded_A_signature = decode(e, n1, decoded_B_private_key) # Decode with A's public key
        original_string += convertToString(decoded_A_signature)

    # Verifying if message if indeed from A
    try:
        final_message = original_string.split(" ")[:-1]
        final_message = ' '.join(final_message)
        public_key_B = original_string.split(" ")[-1]
        extracted_e, extracted_n = list(map(int, public_key_B.split(",")))

        # Check if the message contains the public of B appended at the end
        if extracted_e == public_B_e and extracted_n == public_B_n:
            print(final_message)
        else:
            print("Message not verified.")

    except Exception:
        print("Random Prime Generator could not generate primes. Please Try Again!!") # This exception occurs if the numbers initially generated are not prime

    # Closing the file
    f.close()


if __name__ == "__main__":
    main()
