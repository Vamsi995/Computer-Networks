#include <iostream>
#include <bitset>
#include <string>
#include <dirent.h>
#include <string.h>
#include <vector>
using namespace std;

// Packet Structure
typedef struct IPPacket_t
{
    unsigned char v_hl;                   // Version + Header length: 8 bits
    unsigned char dscp_ecn;               // Type of service field: 8bits
    unsigned short int totalLen;          // Total packet length: 16 bits
    unsigned short int id;                // Fragment ID: 16 bits
    unsigned short int flags_frag_offset; // Flags + fragment offset: 3 + 13 bits
    unsigned char ttl;                    // Time to live field: 8 bits
    unsigned char proto;                  // Protocol: 8 bits
    unsigned short int checksum;          // Checksum: 16 bits
    unsigned char sAddr[4];               // Source IP address: 32 bits
    unsigned char dAddr[4];               // Destination IP address: 32 bits
    unsigned int o1;                      // Header Option
    unsigned int o2;                      // Header Option
    unsigned char data[1024];             // Data
} IPPacket;

// full adder function of binary addition
bool fullAdder(bool b1, bool b2, bool &carry)
{
    bool sum = (b1 ^ b2) ^ carry;
    carry = (b1 && b2) || (b1 && carry) || (b2 && carry);
    return sum;
}

// Function for adding two binaries with overflow re-addition
bitset<16> bitsetAdd(bitset<16> &x, bitset<16> &y)
{
    bool carry = false;
    // bitset to store the sum of the two bitsets
    bitset<16> ans;
    for (int i = 0; i < 16; i++)
    {
        ans[i] = fullAdder(x[i], y[i], carry);
    }

    if (carry == true)
    {
        bitset<16> carryadd("1");
        bitset<16> modifiedAns = bitsetAdd(carryadd, ans);
        return modifiedAns;
    }

    return ans;
}

// Converting types char, short, int to binary
template <typename T>
string toBinary(T value)
{
    int size = sizeof(value) * 8;

    if (size == 8)
    {
        bitset<8> bits(value);
        return bits.to_string();
    }
    else if (size == 16)
    {
        bitset<16> bits(value);
        return bits.to_string();
    }
    else if (size == 32)
    {
        bitset<32> bits(value);
        return bits.to_string();
    }
}

// Converting binary string to int
int binaryToInt(string binaryString)
{
    bitset<32> bits(binaryString);
    return (int)bits.to_ulong();
}

// Converting the 32 bit value into a readable IP address format
string convertToIP(unsigned char *addr, int size)
{

    string final = "";

    /*
    Go through each byte of the 32 bits
        convert this byte into binary
        convert binary into integer
        append to a running string
    */

    for (int i = 0; i < size; i++)
    {
        string temp = toBinary(addr[i]);
        int num = binaryToInt(temp);
        final = final + to_string(num);

        if (i != 3)
            final = final + ".";
    }

    return final;
}

// Prints header details of the given fragment
void printHeaderDetails(IPPacket *fragment, char *filename)
{

    string v_hl = toBinary(fragment->v_hl);
    string version = v_hl.substr(0, v_hl.length() / 2);
    string header_len = v_hl.substr(v_hl.length() / 2, v_hl.length());
    bitset<16> flag_frag_offset(fragment->flags_frag_offset);
    bool rb = flag_frag_offset[13];
    bool df = flag_frag_offset[14];
    bool mf = flag_frag_offset[15];
    flag_frag_offset.reset(13);
    flag_frag_offset.reset(14);
    flag_frag_offset.reset(15);

    string source_ip = convertToIP(fragment->sAddr, 4);
    string dest_ip = convertToIP(fragment->dAddr, 4);

    cout << "filename: " << filename << "\n"
         << "version: " << binaryToInt(version) << "\n"
         << "header length: " << binaryToInt(header_len) << " bytes"
         << "\n"
         << "type of service: " << binaryToInt(toBinary(fragment->dscp_ecn)) << "\n"
         << "fragment length: " << fragment->totalLen << " bytes"
         << "\n"
         << "fragment id: " << fragment->id << "\n"
         << "flags: "
         << "Reserved Bit - " << rb << ", Dont Fragment Bit - " << df << ", More Fragment Bit - " << mf << "\n"
         << "fragment offset: " << flag_frag_offset.to_ulong() << "\n"
         << "ttl: " << binaryToInt(toBinary(fragment->ttl)) << "\n"
         << "proto: " << binaryToInt(toBinary(fragment->proto)) << " - EGP (Exterior Gateway Protocol)"
         << "\n"
         << "checksum: " << fragment->checksum << "\n"
         << "sAddr: " << source_ip << "\n"
         << "dAddr: " << dest_ip << "\n"
         << "option1: " << fragment->o1 << "\n"
         << "option2: " << fragment->o2 << "\n"
         << endl;
}

int main()
{

    char DIR_NAME[256] = "ipfrags/";
    struct dirent *dir = nullptr;
    DIR *dp = opendir(DIR_NAME);

    int counter = 0;

    /*
    Going through all the files in the ipfrags directory and reading them into the structure IPPacket
        if the file is not "." or ".." (these are hidden files in the directory)
            Initialize IPPacket structure
            Open the file using fopen
            read the file content using fread and write it to the IPPacket structure
            Typecast the pointer to the structure with a (short *) pointer (since a short reads 16 bits from the start of the pointer)
            Keep extracting 16 bits this way until the end of the header(i.e for 14 times, since header length is 28 bytes = 28 x 8 bits = (28 x 8)/16 = 14 16-bit blocks ) and store a running binary sum - In this binary sum the overflow bits are added back to the sum
            Compare the sum to 16-bit all ones binary value
                if they are equal then
                    the header is valid
                    print the header details
                else
                    go to the next file
    */

    if (dp != nullptr)
    {
        while ((dir = readdir(dp)))
        {
            char *filename = dir->d_name;
            char filepath[256] = {};
            strcat(filepath, DIR_NAME);
            strcat(filepath, filename);

            if (filename != "." || filename != "..")
            {
                // Opening file
                FILE *file = fopen(filepath, "rb");
                if (!file)
                {
                    printf("Unable to open file\n");
                }
                IPPacket *fragment = (IPPacket *)malloc(sizeof(IPPacket)); // Initializing the structure
                fread(fragment, sizeof(IPPacket), 1, file); // Reading the file

                short *p = (short *)fragment; // Typecasting the pointer into short pointer
                bitset<16> running_sum(0);
               
                /*
                convert the value at the pointer to a binary string
                perform binary addition and store the sum in a running sum
                */
               
                for (int i = 0; i < 14; i++)
                {
                    string binaryString = toBinary(*p);
                    bitset<16> b(binaryString);
                    running_sum = bitsetAdd(running_sum, b);
                    p = p + 1;
                }

                // Validated bits - 16bit all ones
                bitset<16> validate_set(1);
                validate_set.set();

                // Validating the checksum
                if (validate_set.to_string() == running_sum.to_string())
                {
                    counter += 1; // Maintaining count of valid packets
                    // Push into the fragments vector if the header is valid
                    // printHeaderDetails(fragment, filename);
                    cout << filename << endl;
                }

                fclose(file);
            }
        }
        cout << "Number of valid fragments: " << counter << endl;
    }

    closedir(dp);
    return 0;
}