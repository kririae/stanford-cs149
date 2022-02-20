# Stanford CS149

> NOTICE: If your are doing homework, you should not refer to the code in this repo, which might violate the bottom line of **ACADEMIC INTEGRITY**. 

[krr](https://github.com/kririae)'s implementation of [cs149](https://github.com/stanford-cs149). 
Thanks to the original author for open source this project, so that 
we can have access to such a wonderful learning material.

- `asst1/`: Performance Analysis
- `asst2/`: Task Execution Library
- `asst3/`: A Simple CUDA Render
- `asst4/`: Big Graph Processing in OpenMP

## Hints

- `asst1/`: Refer to `asst1/report.ipynb`
- `asst2/`: Coding problem, my implementation is to use a single queue with a lock, which is not the optimal solution.
In *part_b*, I use a offline DAG calculation when calling `sync()`, which can only meet the requirements of abstraction.
- `asst3/`: Consider collect the indices using `prefix_sum` as index, `output[prefix_sum[i]] = i`. 
The render uses the same idea however harder to implement(shared memory version is easier since handling 3D Array in CUDA is disgusting).

## TODO
- [ ] [gemm_extra_credit](https://github.com/stanford-cs149/gemm_extra_credit)
