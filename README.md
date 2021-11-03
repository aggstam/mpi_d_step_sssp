This program implements D-Stepping algorithm introduced by U.Mayers and P.Sanders [1], using SSSP algorith for faster execution[2].
<br>
Algorith can be found in link:
<br>
  https://cs.iupui.edu/~fgsong/LearnHPC/sssp/deltaStep.html
<br>
Graph is read from an input file created by RandomGraph generator by S.Pettie and V.Ramachandran, provided in link:
<br>
  http://www.dis.uniroma1.it/challenge9/code/Randgraph.tar.gz
<br>
Finding shortest path distance for each node occurs in parallel, by using the MPI Standard.
<br>
<br>
Execution command:
<br>
mpiexec -np {number_of_cores} {executable} {d_step} {input_file} {output_file}
<br>
Execution example:
<br>
![terminal](https://user-images.githubusercontent.com/40597439/140077313-6fda7923-57e2-457d-b09e-bbbba5cf2009.png)
<br>
<br>
References:
<br>
[1] Ulrich Meyer and Peter Sanders. Δ-stepping: a parallelizable shortest path algorithm. Journal of
Algorithms 49.1 (2003): 114-152.
<br>
[2] Chakaravarthy, V.T.; Checconi, F.; Petrini, F.; Sabharwal, Y. Scalable Single Source Shortest Path
Algorithms for Massively Parallel Systems, 28th IEEE International Parallel and Distributed Processing
Symposium , pp.889-901, 201
