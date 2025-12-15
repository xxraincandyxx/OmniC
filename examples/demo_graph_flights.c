// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-
//
// Example: Use OmniC graph API to find the minimum number of flight changes
// between two cities (source -> destination) using Dijkstra on a directed
// graph. Every flight has unit cost, so the path with the fewest edges gives
// the minimal number of layovers.

#include <float.h>
#include <omnic/graph.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  size_t from;
  size_t to;
} flight_t;

static void print_path(size_t dst, const size_t* prev, size_t n) {
  size_t* stack = (size_t*)malloc(n * sizeof(size_t));
  if (!stack) {
    fprintf(stderr, "Out of memory while printing path\n");
    return;
  }

  size_t len = 0;
  for (size_t cur = dst; cur != (size_t)-1; cur = prev[cur]) {
    stack[len++] = cur;
  }

  printf("Best route (%zu stops): ", len - 1);
  for (size_t i = 0; i < len; ++i) {
    size_t idx = len - 1 - i;
    printf("%zu", stack[idx]);
    if (idx != 0) {
      printf(" -> ");
    }
  }
  printf("\n");

  free(stack);
}

// Simple O(V^2 + E) Dijkstra specialized for adjacency-list graphs with
// non-negative weights. We set each flight weight to 1 so the shortest path
// corresponds to minimum flight changes.
static bool dijkstra_min_flights(const oc_graph_t* g, size_t src, size_t dst,
                                 double* distance_out, size_t* prev_out) {
  size_t n = oc_graph_num_vertices(g);
  if (src >= n || dst >= n) {
    return false;
  }

  double* dist = (double*)malloc(n * sizeof(double));
  bool* visited = (bool*)calloc(n, sizeof(bool));
  size_t* prev = (size_t*)malloc(n * sizeof(size_t));
  if (!dist || !visited || !prev) {
    free(dist);
    free(visited);
    free(prev);
    return false;
  }

  for (size_t i = 0; i < n; ++i) {
    dist[i] = DBL_MAX;
    prev[i] = (size_t)-1;
  }
  dist[src] = 0.0;

  for (size_t iter = 0; iter < n; ++iter) {
    // Select the unvisited vertex with the smallest distance
    size_t u = (size_t)-1;
    double best = DBL_MAX;
    for (size_t v = 0; v < n; ++v) {
      if (!visited[v] && dist[v] < best) {
        best = dist[v];
        u = v;
      }
    }

    if (u == (size_t)-1 || dist[u] == DBL_MAX) {
      break;  // Remaining vertices unreachable
    }
    visited[u] = true;
    if (u == dst) {
      break;  // Early exit once destination is finalized
    }

    // Relax outgoing edges from u
    if (g->repr_type == OC_GRAPH_REPR_ADJ_LIST ||
        g->repr_type == OC_GRAPH_REPR_HYBRID) {
      oc_graph_edge_t* edge = g->repr.list.lists[u];
      while (edge) {
        size_t v = edge->to;
        double alt = dist[u] + edge->weight;
        if (alt < dist[v]) {
          dist[v] = alt;
          prev[v] = u;
        }
        edge = edge->next;
      }
    } else if (g->repr_type == OC_GRAPH_REPR_ADJ_MATRIX) {
      size_t cap = g->repr.matrix.capacity;
      for (size_t v = 0; v < n; ++v) {
        double w = g->repr.matrix.matrix[(u * cap) + v];
        if (w != 0.0) {
          double alt = dist[u] + w;
          if (alt < dist[v]) {
            dist[v] = alt;
            prev[v] = u;
          }
        }
      }
    }
  }

  bool reachable = dist[dst] != DBL_MAX;
  if (distance_out) {
    *distance_out = dist[dst];
  }
  if (prev_out && reachable) {
    for (size_t i = 0; i < n; ++i) {
      prev_out[i] = prev[i];
    }
  }

  free(dist);
  free(visited);
  free(prev);
  return reachable;
}

int main(void) {
  // Example input: n cities, m directed flights
  const size_t num_cities = 8;
  const flight_t flights[] = {{0, 1}, {0, 2}, {1, 3}, {2, 3}, {2, 4}, {3, 5},
                              {4, 5}, {5, 6}, {6, 7}, {1, 4}, {4, 7}, {0, 6}};
  const size_t m = sizeof(flights) / sizeof(flight_t);
  const size_t src = 0;
  const size_t dst = 7;

  printf("--- Minimum Flight Changes (Dijkstra) ---\n");
  printf("Cities: %zu, Flights: %zu, Source: %zu, Destination: %zu\n\n",
         num_cities, m, src, dst);

  // Build directed graph with adjacency list representation
  oc_graph_t* g = oc_graph_init(num_cities, true, OC_GRAPH_REPR_ADJ_LIST);
  if (!g) {
    fprintf(stderr, "Failed to initialize graph\n");
    return 1;
  }

  for (size_t i = 0; i < m; ++i) {
    oc_graph_insert_edge(g, flights[i].from, flights[i].to, 1.0);
  }

  double distance = 0.0;
  size_t* prev = (size_t*)malloc(num_cities * sizeof(size_t));
  if (!prev) {
    fprintf(stderr, "Failed to allocate prev array\n");
    oc_graph_destroy(g);
    return 1;
  }

  bool found = dijkstra_min_flights(g, src, dst, &distance, prev);
  if (!found) {
    printf("No route found from %zu to %zu\n", src, dst);
  } else {
    printf("Minimum number of flight changes: %.0f\n", distance);
    print_path(dst, prev, num_cities);
  }

  free(prev);
  oc_graph_destroy(g);
  return 0;
}
