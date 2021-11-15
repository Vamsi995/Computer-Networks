#include<stdio.h>
#include<stdlib.h>


int main(int num, char *args[])
{

    int bucket_size = atoi(args[1]);
    float token_rate = atof(args[2]);
    int num_tokens = bucket_size; // Number of tokens in the bucket

    float arrival_time;
    int packet_id;
    int packet_length;

    float previous_transmission_time = 0;
    float transmission_time = 0;

    // Read until the end of file
    while(fscanf(stdin, "%f %d %d", &arrival_time, &packet_id, &packet_length) != EOF) {

        /*
            If the arrival time is greater than previous transmission time
                then find the generated tokens from arrival time and previous transmission time and token rate

            Truncate the number of tokens in the bucket if they exceed

            if number of tokens is greater than the packet length
                then reduce the number of tokens by the packet length
            otherwise
                transmission time is the accumulatin of the amount of time it needs to store up the required amount of tokens

        */

        if(arrival_time > previous_transmission_time) {
            float generated_tokens = (arrival_time - previous_transmission_time) * token_rate;

            if(generated_tokens + num_tokens <= bucket_size) {
                num_tokens += generated_tokens;
            }
            else {
                num_tokens = bucket_size;
            }   

            transmission_time = arrival_time;

        }
          
        if(num_tokens >= packet_length) {
                num_tokens -= packet_length;
        }
        
        else {
            transmission_time += (packet_length - num_tokens)/token_rate;
            num_tokens = 0;
        }

        previous_transmission_time = transmission_time;
        printf("%.2f %d %d\n", transmission_time, packet_id, packet_length);

    }

}