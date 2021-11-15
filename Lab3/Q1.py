# Imports
import numpy as np
from matplotlib import pyplot as plt


# Function to plot the graph, between lamda vs average throughput
def plot(lambdas, average_throughput):
    plt.plot(lambdas, average_throughput, 'o', label="Slotted Aloha")
    plt.xlabel("\u03BB (Number of frames per frame time)")
    plt.ylabel("Average Throughput")
    plt.legend()
    plt.grid(True)
    plt.show()


def main():

    # Variables
    n = 100                             # Number of User Stations
    num_slots = 10000                   # Number of Time Slots
    lamdas = np.arange(0, 1, 0.005)     # Lambda spacing - Number of frames per frame time
    average_throughput = []             # List to store Average Throughputs

    for p in lamdas:                    # Iterating through each lambda value

        successful_transmissions = 0    # Temporary counter to store successful transmissons

        for t in range(num_slots):      # Iterating through the time slots

            created_frame = np.random.uniform(low=0, high=1, size=100) < p / n              # Creating 100 random numbers between 0 and 1
                                                                                            # and finding how many of them are less than the probability value of p/n
                                                                                            # This is similar to tossing a coin 100 times and checking how many of them are heads

            # Checking for collisions
            if np.sum(created_frame) == 1:                                                  # If sum is greater than 1 then there is a collision
                successful_transmissions += 1                                               # Otherwise it is a successful transmission

        average_throughput.append(successful_transmissions / num_slots)                     # Appending throughput to the final list

    plot(lamdas, average_throughput)                                                        # Calling the plot function to plot the average throughput vs lambda


# Script Start
if __name__ == "__main__":
    main()