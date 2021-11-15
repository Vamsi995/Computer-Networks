#include<stdio.h>
#include<stdlib.h>

// Structure for each element in the Queue
typedef struct QNode {

    float transmission_time;
    float arrival_time;
    int packet_length;

}QNode;


// Queue ADT 
typedef struct Queue {
    int front, rear, size;
    int capacity;
    QNode *array;
}Queue;


// Enqueue Function: Takes in queue and Qnode item and puts it in the queue.
void enqueue(Queue* queue, QNode item)
{
    if (queue->size == queue->capacity)
        return;
    queue->size = queue->size + 1;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    
}


// Returns a pointer to the queue ADT
Queue* createQueue(int capacity)
{
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->array = (QNode*)malloc(queue->capacity * sizeof(QNode));
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    return queue;
}



int main(int num, char *args[]) 
{

    int capacity = atoi(args[1]); // Capacity of the queue
    float token_rate = atof(args[2]); // Token rate 
    

    Queue* queue = createQueue(capacity); 

    
    float arrival_time; // Arrival time
    int packet_id;  // Packet id
    int packet_length; // Packet Length

    float previous_transmission_time = 0; // For saving previous transmission time
    float transmission_time = 0; // For saving current transmission time

    // Reading through the file
    while(fscanf(stdin, "%f %d %d", &arrival_time, &packet_id, &packet_length) != EOF) {

        /*
        If packet length is less than the queue capacity
            check if the arrival time is less than the previous transmission time
                Go through the entire queue and check which all packets have transmission time greater than the current one
                sum all their capacities to find the remaining capacity

                if the remaining capacity is greater than the current packet length 
                    then transmission is possible
                otherwise
                    reject the packet and read next arrival

            else(if arrival time is greater than or equal to the previous transmission time)
                then it can be safely transmitted

            enqueue the transmitted packets with the coressponding details.

        */
        
        if(packet_length < capacity) {

            if(arrival_time < previous_transmission_time) {
                
                int running_cap = 0;

                for(int i = queue->front; i < queue->rear + 1; i++) 
                {

                    if(arrival_time < queue->array[i].transmission_time) {
                        running_cap += queue->array[i].packet_length;
                    }

                }

                if(packet_length <= capacity - running_cap){
                    transmission_time = previous_transmission_time + (packet_length/token_rate);
                    previous_transmission_time = transmission_time;
                }

                else
                    continue;
            }
                
            else {
                transmission_time = arrival_time + (packet_length/token_rate); 
                previous_transmission_time = transmission_time;
            }


            
            // Creating new node and pushing it into the packet queue.
            QNode *newnode = (QNode*)malloc(sizeof(QNode));
            newnode->arrival_time = arrival_time;
            newnode->transmission_time = transmission_time;
            newnode->packet_length = packet_length;

            enqueue(queue, *newnode);
            printf("%.2f %d %d\n", transmission_time, packet_id, packet_length);
        }


    }

    
}