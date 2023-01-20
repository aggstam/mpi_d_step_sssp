# mpi_d_step_sssp
This program implements D-Stepping algorithm introduced by U.Mayers and P.Sanders [1], using SSSP algorith for faster execution [2].
<br>
A graphical presentation of the algorith can be found in link:
<br>
  https://cs.iupui.edu/~fgsong/LearnHPC/sssp/deltaStep.html
<br>
Graph is read from an input file created by RandomGraph generator by S.Pettie and V.Ramachandran [3].
<br>
Two implementations are included, one executing the algorithm in serial, and one using the MPI Standard.
<br>
MPI implementation requires *openmpi* package to be installed.

# Usage
Both version can be invocted via the Makefile, or by directly compiling and executing.

## Make usage
### Normal code
```
% make
```
To configure D step value:
```
% make DSTEP={d_step}
```
To include a different input file:
```
% make FILE={file_path}
```
To configure a different output file:
```
% make OUTPUT={file_path}
```

### MPI code
```
% make mpi
```
To configure different how many processes to use:
```
% make mpi PROCESSES={processes}
```
To configure D step value:
```
% make mpi DSTEP={d_step}
```
To include a different input file:
```
% make mpi FILE={file_path}
```
To configure a different output file:
```
% make mpi OUTPUT={file_path}
```

## Direct usage
### Normal code
Compilation:
```
% gcc -o d_step_sssp d_step_sssp.c
```
Execution:
```
% ./d_step_sssp {d_step} {input_file} {output_file}
```

### MPI code
Compilation:
```
% mpicc -lm -o mpi_d_step_sssp mpi_d_step_sssp.c
```
Execution:
```
% mpiexec -np {processes} ./mpi_d_step_sssp {d_step} {input_file} {output_file}
```

<br>
References:
<br>
[1] Ulrich Meyer and Peter Sanders. Δ-stepping: a parallelizable shortest path algorithm. Journal of
Algorithms 49.1 (2003): 114-152.
<br>
[2] Chakaravarthy, V.T.; Checconi, F.; Petrini, F.; Sabharwal, Y. Scalable Single Source Shortest Path
Algorithms for Massively Parallel Systems, 28th IEEE International Parallel and Distributed Processing
Symposium , pp.889-901, 201
<br>
[3] http://www.dis.uniroma1.it/challenge9/code/Randgraph.tar.gz
