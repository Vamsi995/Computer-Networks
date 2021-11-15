import random
import sys
import numpy as np


class Vertex:
    """This class represents the structure of each vertex in the graph.
    :param id: Represents the id of the vertex
    :type id: int
    """

    def __init__(self, id):
        self.id = id
        self.distance = sys.maxsize
        self.parent = None
        self.adj_vertices = {}
        self.visited = False


class Graph:
    """This class represents the structure of a graph. It is implemented using adjacency map.
    """

    def __init__(self):
        """Constructor method
        """
        self._vertexSet = {}

    def add_vertex(self, id):
        """Adding vertex to our vertex set.
        :param id: Identifier of the new vertex being added
        :type id: int
        """
        self._vertexSet[id] = Vertex(id)

    def build_graph(self, filename):
        """Builds the graph from the input file
        :param filename: Name of the input file
        :type filename: str
        """
        with open(filename) as graph_input:
            for line in graph_input:
                u, v, w = line.rstrip().split()
                self.add_edge(int(u), int(v), int(w))
                self.add_edge(int(v), int(u), int(w))

    def get_edge_weight(self, u, v):
        """Gets the edge weight between the vertices u and v
        :param u: vertex in the edge (u,v)
        :type u:  int
        :param v: vertex in the edge(u,v)
        :type v: int
        :return: Weight of the edge (u,v)
        :rtype: int
        """
        return self._vertexSet[u].adj_vertices[v]

    def add_edge(self, u, v, weight=0):
        """Adds an edge to the graph
        :param u: vertex in the edge (u,v)
        :param v: vertex in the edge (u,v)
        :param weight: weight of the edge (u, v)
        """

        if u in self._vertexSet:
            vertex_u = self._vertexSet[u]
            vertex_u.adj_vertices[v] = weight
        else:
            self.add_vertex(u)
            self._vertexSet[u].adj_vertices[v] = weight

    def get_neighbours(self, u):
        """Returns the list of neighbours of the given vertex
        :param u: Vertex whose neighbours are to be returned
        :type u: int
        :return: List of vertex objects
        :rtype: list(<class vertex>)
        """

        l = []
        u = self._vertexSet[u]
        for key, value in u.adj_vertices.items():
            l.append(self._vertexSet[key])
        return l

    def get_vertices(self):
        """Returns the list of vertices in the graph
        :return: List of vertices
        :rtype: list(int)
        """

        list = []
        for key in self._vertexSet:
            list.append(key)
        list.sort()
        return list

    def initialize_dvs(self, n):
        """Initializing the distance vectors with corresponding weights of neighbours
        :param n: Number of nodes
        :return: Distance vectors
        :rtype: numpy.ndarray (2D numpy array)
        """

        """
        Initializing the distance vector to be 2D numpy array where each row represents the distance vector for the
        vertex with the corresponding id
        """
        dv = np.zeros((n, n))
        next_hop_nodes = np.zeros((n, n))

        for u in self.get_vertices():
            dv[u][u] = -1
            next_hop_nodes[u][u] = u
            for v in self.get_neighbours(u):
                dv[u][v.id] = self.get_edge_weight(u, v.id)
                next_hop_nodes[u][v.id] = v.id

        dv = np.where(dv == 0, 10000, dv)
        dv = np.where(dv == -1, 0, dv)

        return dv, next_hop_nodes

    def calculate_distance_vector(self, dv, n, next_hop_nodes):
        """ Computes the distance vectors
        :param dv: Initial distance vector
        :type dv: numpy.ndarray (2D array)
        :param n: number of nodes
        :type n: int
        :return: Distance vectors
        :rtype:  numpy.ndarray (2D array)
        """

        # next_hop_nodes = np.zeros((n, n))
        prev_dv = np.zeros((n, n))

        while not (prev_dv == dv).all():

            prev_dv = dv.copy()

            """Get all the vertices
                Randomly shuffle vertices
                Go through each vertex u
                    Get neighbours of the current vertex u
                    shuffle these neighbours randomly
                    for each vertex v of these neighbours 
                        for each element d of the distance vector of u
                            if d > neighbour_dv + edge_weight(u,v)
                                u[d] = neighbour_dv + edge_weight(u,v)
                                next_hop_node[d] = v.id
            """
            vertices = self.get_vertices()
            random.shuffle(vertices)

            for v_id in vertices:

                neighbours = self.get_neighbours(v_id)

                random.shuffle(neighbours)

                for v in neighbours:
                    c_uv = self.get_edge_weight(v_id, v.id)
                    neighbour_dv = dv[v.id]

                    for j in range(len(dv[v_id])):
                        if dv[v_id][j] > neighbour_dv[j] + c_uv:
                            dv[v_id][j] = neighbour_dv[j] + c_uv
                            next_hop_nodes[v_id][j] = v.id

        return dv, next_hop_nodes

    def print_dvs(self, dv, next_hop_nodes):
        """Print the output
        :param dv: Converged distance vectors
        :type dv: numpy.ndarray (2D array)
        """

        for i in self.get_vertices():
            print()
            print("Vertex {}".format(i))
            print("{:<15}{:<25}{:<15}".format('Vertex', 'Distance Vector', 'Hop Node'))

            for j in self.get_vertices():
                print("{:<15}{:<25}{:<15}".format(str(j), str(int(dv[i][j])), str(int(next_hop_nodes[i][j]))))


def main():
    """Driver Function
    """

    # Parsing Arguments
    arguments = sys.argv[1:]
    num_nodes = int(arguments[0])
    filename = arguments[1]

    graph = Graph()
    graph.build_graph(filename)

    dv, next_hop_nodes = graph.initialize_dvs(num_nodes)
    dv, next_hop_nodes = graph.calculate_distance_vector(dv, num_nodes, next_hop_nodes)

    graph.print_dvs(dv, next_hop_nodes)


if __name__ == "__main__":
    main()