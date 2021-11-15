#include <iostream>
#include <bitset>
#include <string>
#include <dirent.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <bits/stdc++.h>
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

// Comparator for sorting the fragments according to the offset field
bool cmp(const IPPacket *lhs, const IPPacket *rhs)
{
    bitset<16> flag_frag_lhs(lhs->flags_frag_offset);
    flag_frag_lhs.reset(15);
    flag_frag_lhs.reset(14);
    flag_frag_lhs.reset(13);

    bitset<16> flag_frag_rhs(rhs->flags_frag_offset);
    flag_frag_rhs.reset(15);
    flag_frag_rhs.reset(14);
    flag_frag_rhs.reset(13);

    return (unsigned short)flag_frag_lhs.to_ulong() < (unsigned short)flag_frag_rhs.to_ulong(); // here go your sort conditions
}

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

    // In case of overflow, add the overflow back to the 16 bit number and keep doing this until there is no overflow
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

int main()
{

    vector<IPPacket *> fragments;

    char DIR_NAME[256] = "ipfrags/";
    struct dirent *dir = nullptr;
    DIR *dp = opendir(DIR_NAME);

    /*
    Going through all the files in the ipfrags directory and reading them into the structure IPPacket
        if the file is not "." or ".." (these are hidden files in the directory)
            Initialize IPPacket structure
            Open the file using fopen
            read the file content using fread and write it to the IPPacket structure
            Typecaset the pointer to the structure with a (short *) pointer (since a short reads 16 bits from the start of the pointer)
            Keep extracting 16 bits this way until the end of the header(i.e for 14 times, since header length is 28 bytes = 28 x 8 bits = (28 x 8)/16 = 14 16-bit blocks ) and store a running binary sum - In this binary sum the overflow bits are added back to the sum
            Compare the sum to 16-bit all ones binary value
                if they are equal then
                    the header is valid
                    store the fragment into the fragments vector
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
                fread(fragment, sizeof(IPPacket), 1, file);                // Reading the file

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
                    // Push into the fragments vector if the header is valid
                    fragments.push_back(fragment);
                }

                fclose(file);
            }
        }
    }

    closedir(dp);

    // This map stores the fragment ID and the corresponding fragments in a vector
    map<short, vector<IPPacket *>> packetMap;

    /*
    Go through all the fragments in the fragments vector
        if the fragment id is present in the map
            then push it to the vector corresponding to the id
        else
            create a new key value pair of (id, vector<IPPacket*>)
            push the fragment into this vector
    */

    for (int i = 0; i < fragments.size(); i++)
    {
        unsigned short id = fragments[i]->id;

        if (packetMap.count(id) == 0)
        {
            vector<IPPacket *> temp;
            packetMap.insert({id, temp});
            packetMap[id].push_back(fragments[i]);
        }
        else
        {
            packetMap[id].push_back(fragments[i]);
        }
    }

    cout << "\nNumber of Packets: " << packetMap.size() << "\n"
         << endl;

    /*
    for each (id, vector<IPPacket*>) pair in the map


        sort all the fragments in vector<IPPacket*> according to the offset value of each fragment

        for each fragment in the vector<IPPacket*>

            calculate the datalen = total packet length - header length
            skip to the end of the header using an int pointer (incrementing the pointer by 7 times skips 7*4 bytes = 28 bytes (header length))
            capture datalen amount of bytes (typecast the pointer into a char pointer since each char is of 1 byte, increment this pointer by datalen times and on each increment save the character)
            concatenate the captured data to a running string

        print the running string
    */

    for (auto i = packetMap.begin(); i != packetMap.end(); i++)
    {

        unsigned short id = i->first;
        vector<IPPacket *> fragments = i->second;

        int total_packet_size = 0;
        std::sort(fragments.begin(), fragments.end(), cmp);
        string message = "";

        for (int i = 0; i < fragments.size(); i++)
        {
           

            IPPacket *fragment = fragments[i];
            unsigned short data_len = fragment->totalLen - 28; // 28 bytes is the header size
            total_packet_size += data_len;

            if (data_len == 0)
                continue;

            int *p = (int *)fragment;

            for (int i = 0; i < 7; i++)
            {
                p = p + 1;
            }

            char *ptr = (char *)p;

            for (int j = 0; j < data_len; j++)
            {
                message.push_back(*ptr);
                ptr = ptr + 1;
            }
        }

        cout << "Packet ID: " << id << "\n"
             << "Packet Size: " << total_packet_size + 28 << "\n" // Packet size is total data length + IP header length
             << "Number of Fragments: " << fragments.size() << "\n"
             << "Message: " << message << "\n"
             << endl;
    }

    return 0;
}