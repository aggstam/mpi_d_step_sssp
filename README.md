This program implements D-Stepping algorithm introduced by U.Mayers and P.Sanders [1], using SSSP algorith for faster execution[2].
Algorith can be found in link:
  https://cs.iupui.edu/~fgsong/LearnHPC/sssp/deltaStep.html
Graph is read from an input file created by RandomGraph generator by S.Pettie and V.Ramachandran, provided in link:
  http://www.dis.uniroma1.it/challenge9/code/Randgraph.tar.gz
Finding shortest path distance for each node occurs in parallel, by using the MPI Standard.
Execution command:
mpiexec -np {number_of_cores} {executable} {d_step} {input_file} {output_file}
Execution example:
![terminal](https://user-images.githubusercontent.com/40597439/140077313-6fda7923-57e2-457d-b09e-bbbba5cf2009.png)

References:
[1] Ulrich Meyer and Peter Sanders. Δ-stepping: a parallelizable shortest path algorithm. Journal of
Algorithms 49.1 (2003): 114-152.
[2] Chakaravarthy, V.T.; Checconi, F.; Petrini, F.; Sabharwal, Y. Scalable Single Source Shortest Path
Algorithms for Massively Parallel Systems, 28th IEEE International Parallel and Distributed Processing
Symposium , pp.889-901, 201
