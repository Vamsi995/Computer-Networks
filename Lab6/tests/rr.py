# Please use Python 3.6 or 3.6+ for running this code
import sys


class Packet:
    """Packet Class that stores the information of a singlel packet
    """

    def __init__(self, arrival_time, packet_id, packet_length):
        """Constructor Method
        """
        self.arrival_time = arrival_time
        self.packet_id = packet_id
        self.packet_length = packet_length


def main():
    """Driver Function
    """

    # Program Arguments
    args = sys.argv[1:]
    rate = float(args[0])
    file_name = args[1]

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
            queue[queue_id].append(Packet(arrival_time, packet_id, packet_length))

    start_round = next(iter(queue))  # First queue to start the round robin
    queue = dict(sorted(queue.items(), key=lambda item: item[0]))  # Sort the dictionary based on the queue number.
    max_queue = len(queue)  # Number of queues

    prev_transmission_time = queue[start_round][0].arrival_time  # Previous transmission time set to the arrival time of the first packet
    transmission_time = 0
    counter = 0  # Counter for identifying arrival time being greater than previous transmission time in all the queues
    min_key = 0

    # Loop until all the packets are transmitted
    while num_packets > 0:

        # Perform round robin rounds starting from the first queue to the last queue in the dictionary
        """for each queue in the dictionary
                if the queue is empty then continue
                
                if packet arrival time = previous transmission time
                    then transmisison time = arrival time + packet length/rate
                
                if packet arrival time > previous transmission time 
                    then track the number of skipping queues with a counter
                    if it manages to skip over all the queues 
                        then find the min arrival packet among all queues and start the round from that packet 
                
                if packet arrival time < previous transmission time
                    then transmission time = previous transmission time + packet length/rate
                
        """

        for key in range(start_round, max_queue + 1):

            # If queue is empty then skip
            if len(queue[key]) != 0:
                current_packet = queue[key][0]
            else:
                counter += 1
                continue

            if current_packet.arrival_time == prev_transmission_time:
                # Updating the transmission time
                transmission_time = prev_transmission_time + (current_packet.packet_length / rate)
                print("{:.2f} {}".format(transmission_time, current_packet.packet_id))
                prev_transmission_time = transmission_time

                # Dequeing the element from the queue
                current_val = queue[key]
                current_val.pop(0)
                queue[key] = current_val
                del current_packet
                num_packets -= 1

            elif current_packet.arrival_time > prev_transmission_time:
                counter += 1
                continue

            else:
                # Updating the transmission time
                transmission_time = prev_transmission_time + (current_packet.packet_length / rate)
                print("{:.2f} {}".format(transmission_time, current_packet.packet_id))
                prev_transmission_time = transmission_time

                # Dequeing the element from the queue
                current_val = queue[key]
                current_val.pop(0)
                queue[key] = current_val
                del current_packet
                num_packets -= 1

        if counter == max_queue:
            # Finding the minimal arrival time packet among the front of all the queues
            min_arrival_time = sys.maxsize
            min_key = 0

            """Finding minimum among all the front of queues
            """
            for key, value in queue.items():
                if len(value) != 0 and value[0].arrival_time < min_arrival_time:
                    min_arrival_time = value[0].arrival_time
                    min_key = key
            prev_transmission_time = min_arrival_time
            # Starting the round at the queue which has the minimum arrival time
            start_round = min_key
            counter = 0
        else:
            start_round = 1
            counter = 0


if __name__ == "__main__":
    main()