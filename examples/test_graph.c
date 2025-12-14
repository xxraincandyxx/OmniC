// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <omnic/graph.h>
#include <stdio.h>

// Visitor callback for DFS/BFS
void visit_vertex(size_t vertex) { printf("  Visited vertex: %zu\n", vertex); }

int main(void) {
  printf("--- OmniC Graph Example ---\n\n");

  // Test 1: Create a directed graph with adjacency list
  printf("Test 1: Creating directed graph (adjacency list)\n");
  oc_graph_t* g = oc_graph_init(5, true, OC_GRAPH_REPR_ADJ_LIST);
  if (!g) {
    fprintf(stderr, "Failed to create graph.\n");
    return 1;
  }
  printf("Graph created: %zu vertices, %zu edges\n", oc_graph_num_vertices(g),
         oc_graph_num_edges(g));
  printf("Is directed: %s\n\n", oc_graph_is_directed(g) ? "Yes" : "No");

  // Test 2: Add edges
  printf("Test 2: Adding edges\n");
  oc_graph_insert_edge(g, 0, 1, 1.0);
  oc_graph_insert_edge(g, 0, 2, 2.0);
  oc_graph_insert_edge(g, 1, 3, 3.0);
  oc_graph_insert_edge(g, 2, 3, 4.0);
  oc_graph_insert_edge(g, 3, 4, 5.0);
  printf("Added 5 edges. Total edges: %zu\n\n", oc_graph_num_edges(g));

  // Test 3: Check edge existence
  printf("Test 3: Checking edge existence\n");
  if (oc_graph_find_edge(g, 0, 1)) {
    printf("Edge (0 -> 1) exists\n");
  }
  if (!oc_graph_find_edge(g, 1, 0)) {
    printf("Edge (1 -> 0) does not exist (correct for directed graph)\n");
  }
  printf("\n");

  // Test 4: Get edge weight
  printf("Test 4: Getting edge weights\n");
  double weight;
  if (oc_graph_get_edge_weight(g, 0, 2, &weight)) {
    printf("Weight of edge (0 -> 2): %.1f\n", weight);
  }
  if (oc_graph_get_edge_weight(g, 3, 4, &weight)) {
    printf("Weight of edge (3 -> 4): %.1f\n", weight);
  }
  printf("\n");

  // Test 5: DFS traversal
  printf("Test 5: DFS traversal from vertex 0\n");
  oc_graph_dfs(g, 0, visit_vertex);
  printf("\n");

  // Test 6: BFS traversal
  printf("Test 6: BFS traversal from vertex 0\n");
  oc_graph_bfs(g, 0, visit_vertex);
  printf("\n");

  // Test 7: Remove edge
  printf("Test 7: Removing edge (1 -> 3)\n");
  if (oc_graph_rm_edge(g, 1, 3)) {
    printf("Edge removed. Total edges: %zu\n", oc_graph_num_edges(g));
  }
  printf("\n");

  // Test 8: Add vertex
  printf("Test 8: Adding a new vertex\n");
  size_t new_vertex = oc_graph_insert_vertex(g);
  printf("New vertex index: %zu. Total vertices: %zu\n", new_vertex,
         oc_graph_num_vertices(g));
  oc_graph_insert_edge(g, 4, new_vertex, 6.0);
  printf("Added edge (4 -> %zu). Total edges: %zu\n\n", new_vertex,
         oc_graph_num_edges(g));

  // Test 9: Undirected graph
  printf("Test 9: Creating undirected graph (adjacency matrix)\n");
  oc_graph_destroy(g);
  g = oc_graph_init(4, false, OC_GRAPH_REPR_ADJ_MATRIX);
  if (!g) {
    fprintf(stderr, "Failed to create undirected graph.\n");
    return 1;
  }
  oc_graph_insert_edge(g, 0, 1, 1.0);
  oc_graph_insert_edge(g, 1, 2, 2.0);
  oc_graph_insert_edge(g, 2, 3, 3.0);
  printf("Created undirected graph with 3 edges (6 connections)\n");
  printf("Edge (0 -> 1) exists: %s\n",
         oc_graph_find_edge(g, 0, 1) ? "Yes" : "No");
  printf("Edge (1 -> 0) exists: %s\n",
         oc_graph_find_edge(g, 1, 0) ? "Yes" : "No");
  printf("Total edges: %zu\n\n", oc_graph_num_edges(g));

  // Test 10: Hybrid representation
  printf("Test 10: Testing hybrid representation\n");
  oc_graph_destroy(g);
  g = oc_graph_init(10, true, OC_GRAPH_REPR_HYBRID);
  if (!g) {
    fprintf(stderr, "Failed to create hybrid graph.\n");
    return 1;
  }
  // Add sparse edges (should use list)
  for (size_t i = 0; i < 5; ++i) {
    oc_graph_insert_edge(g, i, i + 1, 1.0);
  }
  printf("Hybrid graph: %zu vertices, %zu edges\n", oc_graph_num_vertices(g),
         oc_graph_num_edges(g));
  printf("\n");

  // Cleanup
  oc_graph_destroy(g);
  printf("Graph destroyed.\n");
  printf("\nAll tests completed successfully!\n");

  return 0;
}
