#include "page_rank.h"

#include <omp.h>
#include <stdlib.h>

#include <cmath>
#include <utility>
#include <functional>
#include <numeric>

#include "../common/CycleTimer.h"
#include "../common/graph.h"

// pageRank --
//
// g:           graph to process (see common/graph.h)
// solution:    array of per-vertex vertex scores (length of array is
// num_nodes(g)) damping:     page-rank algorithm's damping parameter
// convergence: page-rank algorithm's convergence threshold
//
void pageRank(Graph g, double *solution, double damping, double convergence) {
  // initialize vertex weights to uniform probability. Double
  // precision scores are used to avoid underflow for large graphs

  int numNodes = num_nodes(g);
  double equal_prob = 1.0 / numNodes;
  for (int i = 0; i < numNodes; ++i) {
    solution[i] = equal_prob;
  }

  /*
     CS149 students: Implement the page rank algorithm here.  You
     are expected to parallelize the algorithm using openMP.  Your
     solution may need to allocate (and free) temporary arrays.

     Basic page rank pseudocode is provided below to get you started:

     // initialization: see example code above
     score_old[vi] = 1/numNodes;

     while (!converged) {

       // compute score_new[vi] for all nodes vi:
       score_new[vi] = sum over all nodes vj reachable from incoming edges
                          { score_old[vj] / number of edges leaving vj  }
       score_new[vi] = (damping * score_new[vi]) + (1.0-damping) / numNodes;

       score_new[vi] += sum over all nodes v in graph with no outgoing edges
                          { damping * score_old[v] / numNodes }

       // compute how much per-node scores have changed
       // quit once algorithm has converged

       global_diff = sum over all nodes vi { abs(score_new[vi] - score_old[vi])
     }; converged = (global_diff < convergence)
     }

   */

  double *score_new = (double *) malloc(sizeof(double) * numNodes);
  int *node_v = (int *) malloc(sizeof(int) * numNodes);
  int *node_bk = (int *) malloc(sizeof(int) * numNodes);
  bool converged = false;
  while (!converged) {
    for (int vi = 0; vi < numNodes; ++vi) {
      score_new[vi] = 0.0;

      const Vertex *start = incoming_begin(g, vi);
      const Vertex *end = incoming_end(g, vi);
      for (const Vertex *v = start; v != end; v++) {
        // vj is the vertex `reachable from incoming edge`
        const int vj = *v;
        score_new[vi] += solution[vj] / outgoing_size(g, vj);
      }

      score_new[vi] = (damping * score_new[vi]) + (1.0 - damping) / numNodes;

      // collect nodes with no outgoing edges (in parallel)
      if (outgoing_size(g, vi) == 0)
        node_v[vi] = 1;
      else
        node_v[vi] = 0;
    }

    memcpy(node_bk, node_v, sizeof(int) * numNodes);
    std::exclusive_scan(node_bk, node_bk + numNodes);
    // Collect index inplace
    for (int vi = 0; vi < numNodes; ++vi) {
      if (node_v[vi] == 1) {
        node_v[node_bk[vi]] = vi + 1; // +1 as a mark
      }
    }

    for (int vi = 0; vi < numNodes; ++vi) {
      for (int v_index = 0; node_v[v_index] != 0; ++v_index) {
        const int v = node_v[v_index] - 1;
        
      }
    }
  }

  free(score_new);
}
