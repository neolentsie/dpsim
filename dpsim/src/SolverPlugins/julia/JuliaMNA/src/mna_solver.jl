using SparseArrays

# Housekeeping
function mna_init()
end

function mna_cleanup()
    end

# Solving Logic
function mna_decomp(sparse_mat; overwrite=true)
    println("lu decomposition with Julia ExampleLIb...")
    lu_decomp = SparseArrays.lu(sparse_mat)
    return lu_decomp
end

function mna_solve(system_matrix, rhs)
    # println("solve with Julia ExampleLib...")
    return system_matrix \ rhs
end