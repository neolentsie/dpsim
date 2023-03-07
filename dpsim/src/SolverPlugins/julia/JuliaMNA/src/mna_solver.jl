using SparseArrays

# Housekeeping
function mna_init()
end

function mna_cleanup()
end

# Solving Logic
function mna_decomp(sparse_mat, accelerators)
    if "CUDA" in accelerators
        return mna_decomp_gpu(sparse_mat)
    else
        return mna_decomp_cpu(sparse_mat)
    end
end

function mna_decomp_gpu(sparse_mat)
    matrix = CuArray(sparse_mat)
    return matrix
end

function mna_decomp_cpu(sparse_mat)
    lu_decomp = SparseArrays.lu(sparse_mat)
    return lu_decomp
end

function mna_solve(system_matrix, rhs)
    # println("solve with Julia ExampleLib...")
    if "CUDA" in accelerators
        return system_matrix \ CuVector(rhs)
    else
        return system_matrix \ rhs
    end
end