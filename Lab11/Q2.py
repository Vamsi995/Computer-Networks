def convertToInt(st):
    """Converts string -> binary -> integer
    :param st: str
    :return: int
    """

    byte_array = st.encode()
    binary_int = int.from_bytes(byte_array, "big")
    return binary_int


def breakString(max_size, e, n):
    """Breaks the text from message.txt into chunks that are processable by the prime numbers
    :param max_size: int (maximum bits of each chunk)
    :param e: int (part of public key of B)
    :param n: int (part of public key of B)
    :return: list (list of strings)
    """

    # Read from message.txt
    f = open("message.txt", "r")
    text = f.read()
    f.close()

    # Adding public key of B into the text of A
    # text += " " + str(e) + "," + str(n)

    str_list = []
    temp = ""

    if n > convertToInt(text):
        str_list.append(text)
        return str_list
    else:
        for i in range(len(text)):
            temp += text[i]
            if (i + 1) % max_size / 8 == 0:
                if temp != "":
                    str_list.append(temp)
                temp = ""
                continue
            if i == len(text) - 1:
                if temp != "":
                    str_list.append(temp)

    return str_list


def encode(e, n, m):
    """ Encodes the message m
    :param e: int (e is part of the public key)
    :param n: int (n is part of the public key)
    :param m: int (m is the message)
    :return: int (encoded message)
    """
    enc_message = pow(m, e, n)
    return enc_message


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
    e, n1 = read_file("B.pub")
    d, n2 = read_file("A.pri")

    # Breaking string into chunks so that it can be converted into integers without conflict with n1
    max_size = (n1.bit_length() + 1) / 32
    parsed_string = breakString(max_size, e, n1)

    # Clearing secret.txt for reuse and opening again for append mode
    f = open("secret.txt", "w")
    f.write("")
    f.close()
    f = open("secret.txt", "a")

    for st in parsed_string:
        plaintext = convertToInt(st)  # Converting text to integer
        print(plaintext)
        sign = encode(d, n2, plaintext)  # Encoding with private key of A
        enc = encode(e, n1, sign)  # Encoding with public key of B

        # Writing to the file secret.txt by using space as a delimiter to differentiate chunks of text
        if st == parsed_string[len(parsed_string) - 1]:
            f.write(str(enc))
        else:
            f.write(str(enc))
            f.write(" ")

    print("Encryption saved to secret.txt")
    # Closing file pointer
    f.close()


if __name__ == "__main__":
    main()
