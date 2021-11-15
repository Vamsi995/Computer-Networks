# Please use Python 3.6 or 3.6+ for running this code
import sys


class Packet:
    """Packet Class that stores the information of a singlel packet
    """

    def __init__(self, arrival_time, packet_id, queue_id, packet_length):
        """Constructor Method
        """
        self.arrival_time = arrival_time
        self.packet_id = packet_id
        self.packet_length = packet_length
        self.queue_id = queue_id


def main():
    """Driver Function
    """

    # Program Arguments
    args = sys.argv[1:]
    rate = float(args[0])
    weights = list(map(float, args[1:5]))  # Weights are integers since it is assumed to be packet wise scheduling
    file_name = args[5]

    num_packets = 0

    # Implementing multi level queues with dictionary
    queue = {}

    # Reading the file
    with open(file_name) as file:

        # Storing all the packets in the queue based on their queue_id
        for line in file:
            arrival_time, packet_id, queue_id, packet_length = line.rstrip().split(" ")
            arrival_time, packet_id, queue_id, packet_length = float(arrival_time), int(packet_id), int(queue_id), int(
                packet_length)

            if (queue_id not in queue):
                queue[queue_id] = []

            num_packets += 1
            queue[queue_id].append(Packet(arrival_time, packet_id, queue_id, packet_length))

    start_round = next(iter(queue))  # First queue to start the round robin
    queue = dict(sorted(queue.items(), key=lambda item: item[0]))  # Sort the dictionary based on the queue number.
    max_queue = len(queue)  # Number of queues

    prev_transmission_time = queue[start_round][
        0].arrival_time  # Previous transmission time set to the arrival time of the first packet
    transmission_time = 0
    min_key = 0

    prev_finish_time = [0.0] * max_queue  # Finish times of all the queues

    prev_packet_id = 0  # Temporary variable used to solve the priority between packets having the same finish time

    # Loop until all the packets are transmitted
    while num_packets > 0:

        """for each queue in the dictionary
               
               if queue is empty then skip
               
               if previous transmission time < current transmission time 
                    then skip (since this packet hasn't arrived into the queue yet)
                
                find finish time of the current packet = max (current arrival time, previous finish time of the respective queue) + current packet length/weight of the corresponding queue
                
                find the minimum among all these finish times and if two finish times are equal break the tie with packet id
                
            
            Dequeue the packet with the smallest finish time
            Update the transmission time = max(arrival time, previous transmission time) + packet length/rate
            Update the previous finish time of that queue
            
        """

        min_finish_time = sys.maxsize
        min_key = 0
        for key in range(1, max_queue + 1):

            if len(queue[key]) == 0:
                continue

            current_packet = queue[key][0]  # Front of the queue with id as key

            if prev_transmission_time < current_packet.arrival_time:
                continue

            finish_time = max(current_packet.arrival_time, prev_finish_time[key - 1]) + (current_packet.packet_length / weights[key-1])

            # Finding minimum finish time
            if finish_time < min_finish_time:
                min_finish_time = finish_time
                min_key = key
                prev_packet_id = current_packet.packet_id

            # For breaking the tie if we have two packets with the same finish time
            if finish_time == min_finish_time:
                if prev_packet_id > current_packet.packet_id:
                    prev_packet_id = current_packet.packet_id
                    min_key = key

        if num_packets == 0:
            break


        current_packet = queue[min_key][0]
        prev_finish_time[min_key - 1] = min_finish_time  # Updating previous finish time
        transmission_time = max(prev_transmission_time, current_packet.arrival_time) + (current_packet.packet_length/rate) # Updating transmission time
        prev_transmission_time = transmission_time
        print("{:.2f} {} {}".format(transmission_time, current_packet.packet_id, current_packet.queue_id))

        # Dequeueing the packet from the queue
        del current_packet
        current_val = queue[min_key]
        current_val.pop(0)
        queue[min_key] = current_val
        num_packets -= 1


if __name__ == "__main__":
    main()