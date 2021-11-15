# Imports
import numpy as np
from matplotlib import pyplot as plt


# Creating Frames of n users
def create_frames(p, n):
    a = np.random.uniform(low=0, high=1, size=100) <= (p / n)
    a.astype(int)
    return a


# Plotting the graph
def plot(lamdas, average_throughput):
    plt.plot(lamdas, average_throughput[:len(lamdas)], 'r', label="p = 0.01")
    plt.plot(lamdas, average_throughput[len(lamdas):], 'g', label="p = 0.5")

    plt.xlabel("\u03BB (Number of frames per frame time)")
    plt.ylabel("Average Throughput")
    plt.legend()
    plt.grid(True)
    plt.show()


# Transmitting the top frames of the queue from each station
def transmit(queues, p1):
    num_transmit = queues[queues > 0]

    probs = np.random.uniform(low=0, high=1, size=len(num_transmit)) <= p1
    probs.astype(int)

    queues[queues > 0] = probs

    return queues


def main():
    # Variables
    np.random.seed(142)  # Random Seed
    n = 100  # Number of User Stations
    lam = np.linspace(0, 1, 50)  # Lambda spacing - Number of frames per frame time
    num_slots = 1000  # Number of time slots
    average_throughput = []  # Average Throughput list
    persistence = [0.01, 0.5]  # p - persistence values

    # Looping through the persistence
    for p1 in persistence:

        # Initializing the queue for each station
        curr_q = np.zeros(n)
        counts = np.zeros(n)

        for p in lam:

            successful_transmissons = 0  # Number of successful transmissions
            wait_flag = 0  # Wait flag is for counting the 3 slots
            busy_flag = 0  # Busy flag is for indicating busy channel

            for i in range(num_slots):

                # Creating frames from all the n channels
                curr_q = create_frames(p, n)
                counts = counts + curr_q
                curr_q = np.where(counts > 0, 1, 0)

                # If wait flag is a multiple of 3 it indicates that the busy line has been there for 3 time slots.
                if wait_flag % 3 == 0:
                    transmission_out = transmit(curr_q,
                                                p1)  # Transmission output from the queue values from all the stations

                    # If sum of the transmission output is equal to 1 means there is no collision
                    if np.sum(transmission_out) == 1:
                        successful_transmissons += 1
                        counts = counts - transmission_out
                        wait_flag = 0
                        busy_flag = 1

                    # If sum is zero then there is no transmission - skip time slot
                    elif np.sum(transmission_out) == 0:
                        continue

                    # If sum is greater than 1 it means there is a collision - wait for 3 time slots by setting the busy flag
                    else:
                        busy_flag = 1

                # If busy then increment wait flag
                if busy_flag == 1:
                    wait_flag += 1

            # Append the current lambdas throughput value into average throughput list
            average_throughput.append(successful_transmissons / num_slots)

    # Calling the plot
    plot(lam, average_throughput)


if __name__ == "__main__":
    main()
