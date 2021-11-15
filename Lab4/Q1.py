import sys


class PriorityQueue:
    """This is priority queue class which uses a dictionary as a priority queue where priority is given to the key with
        the minimum value. The dictionary stores key-value pairs of <Vertex: Distance from source>
    """

    def __init__(self):
        """Constructor method
        """
        self.queue = {}

    def put(self, id, dist):
        """Inserts item into the priority queue
        :param id: Vertex ID
        :type id: int

        :param dist: Distance from source
        :type dist: int
        """
        self.queue[id] = dist
        self.queue = dict(sorted(self.queue.items(), key=lambda item: item[1]))

    def get(self, id):
        """Gets the distance of the corresponding vertex from the queue
        :param id: Vertex ID
        :type id: int

        :return: Distance from source
        :rtype: int
        """
        return self.queue[id]

    def empty(self):
        """Checks if the queue is empty
        :return: Boolean flag indicating whether queue is empty
        :rtype: bool
        """
        return len(self.queue) == 0

    def extract_min(self):
        """Extracting the key associated with minimum value from the queue
        :return: vertex id, distance from source
        :rtype: tuple
        """

        min_val = (0, 0)

        for key, value in self.queue.items():
            min_val = (key, value)
            break

        self.queue.pop(min_val[0])

        return min_val[0], min_val[1]


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
        return list

    def shortest_path(self, u):
        """Returns the shortest path in the graph after the Dijkstra's algorithm
        :param u: Destination vertex
        :type u: int
        :return: String of space separated vertices denoting the path
        :rtype: str
        """

        list = [str(u.id)]
        temp_parent = u.parent
        while temp_parent is not None:
            parent_vertex = self._vertexSet[temp_parent]
            list.append(str(parent_vertex.id))
            temp_parent = parent_vertex.parent

        list.reverse()

        return ' '.join(list)

    def dijkstra(self, source):
        """Dijkstra's Algorithm
        :param source: Source vertex
        :type source: int
        :return: A map containing keys as the vertices and the values as the distance from the source
        :rtype: dict
        """

        # Initializing the output map, the queue, and source vertex distance to 0
        distance_map = {}
        queue = PriorityQueue()
        self._vertexSet[source].distance = 0

        # Pushing all the vertices and their distances to the queue
        for i in self.get_vertices():
            queue.put(self._vertexSet[i].id, self._vertexSet[i].distance)

        while not queue.empty():

            v_id, dist = queue.extract_min()  # Extract minimum distance vertex from the queue

            curr_vertex = self._vertexSet[v_id]
            curr_vertex.visited = True  # Mark the vertex as visited
            curr_vertex.distance = dist

            distance_map[v_id] = dist  # Append this extracted vertex into the final output map

            """For all the neighbours of this vertex
                    If the neighbour is not visited and the relaxation inequality holds then
                        set the new distance for the vertex and alter its distance value in the queue
                        set the neighbours parent to the current vertex
            """
            for v in self.get_neighbours(v_id):

                if (not v.visited) and queue.get(v.id) > self.get_edge_weight(curr_vertex.id,
                                                                              v.id) + curr_vertex.distance:
                    new_dist = self.get_edge_weight(curr_vertex.id, v.id) + curr_vertex.distance
                    queue.put(v.id, new_dist)
                    v.parent = v_id

        return distance_map

    def print_output(self, distance_map):
        """Prints the output
        :param distance_map: Contains the output from the dijkstras algorithm
        :type distance_map: dict
        """

        print("{:<15} {:<30} {:<15}".format('Vertex', 'Cost', 'Path'))

        for key, value in sorted(distance_map.items()):
            path = self.shortest_path(self._vertexSet[key])

            if value == sys.maxsize:
                value = "infinity"
                path = "No Route"

            print("{:<15} {:<30} {:<15}".format(key, value, path))


def main():
    """Driver Function
    """

    # Parsing arguments
    arguments = sys.argv[1:]
    num_nodes = int(arguments[0])
    source_node = int(arguments[1])
    filename = arguments[2]

    graph = Graph()
    graph.build_graph(filename)

    distance_map = graph.dijkstra(source_node)
    graph.print_output(distance_map)


if __name__ == "__main__":
    main()
