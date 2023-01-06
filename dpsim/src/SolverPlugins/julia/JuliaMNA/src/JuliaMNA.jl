# Main module providing ccallable wrapper functions for the mna solver library
module JuliaMNA

export get_mna_plugin, init, decomp, solve, cleanup, log
export dpsim_csr_matrix

using SparseMatricesCSR

include("mna_solver.jl")


# Julia representation of DPsim structs
mutable struct dpsim_csr_matrix
    values::Ptr{Cdouble}    # size: nnz
    rowIndex::Ptr{Cint}     # size: row_number + 1
    colIndex::Ptr{Cint}     # size: nnz
    row_number::Cint        # number of rows of the matrix
    nnz::Cint               # number of non-zero elements in the matrix
end

# Function dummies
Base.@ccallable function init(matrix_ptr::Ptr{dpsim_csr_matrix})::Cint
    # println(matrix_ptr)
    mat_ptr = unsafe_load(matrix_ptr)
    @debug "mat_ptr = $mat_ptr"

    sparse_mat = SparseMatrixCSR{0}(
        mat_ptr.row_number, # Number of rows
        mat_ptr.row_number, # Matrices are square
        unsafe_wrap(Array, mat_ptr.rowIndex, mat_ptr.row_number+1), # Row pointers
        unsafe_wrap(Array, mat_ptr.colIndex, mat_ptr.nnz), # Column indices
        unsafe_wrap(Array, mat_ptr.values, mat_ptr.nnz) # Non-zero values
    )
    @debug "sparse_mat = $(dump(sparse_mat))"

    lu_mat = mna_decomp(sparse_mat)
    global system_matrix = lu_mat

    # println(sparse_mat)
    # println(lu_mat)
    mna_init()
    return 0
end

Base.@ccallable function decomp(matrix_ptr::Ptr{dpsim_csr_matrix})::Cint
    mat_ptr = unsafe_load(matrix_ptr)
    @debug "mat_ptr = $mat_ptr"

    sparse_mat = SparseMatrixCSR{0}(
        mat_ptr.row_number, # Number of rows
        mat_ptr.row_number, # Matrices are square
        unsafe_wrap(Array, mat_ptr.rowIndex, mat_ptr.row_number+1), # Row pointers
        unsafe_wrap(Array, mat_ptr.colIndex, mat_ptr.nnz), # Column indices
        unsafe_wrap(Array, mat_ptr.values, mat_ptr.nnz) # Non-zero values
    )
    @debug "sparse_mat = $(dump(sparse_mat))"

    lut_mat = mna_decomp(sparse_mat)
    global system_matrix = lu_mat
    
    return 0
end

Base.@ccallable function solve(rhs_values_ptr::Ptr{Cdouble}, lhs_values_ptr::Ptr{Cdouble})::Cint
    # try
    #     dim = system_matrix.parent.m # Matrix is quadratic, so we can use m or n
    # catch e
    #     println("Could not read dimensions of system matrix. ", e)
    #     return -1
    # end

    # open("rhs.txt", "w") do io
    #     write(io, string(unsafe_wrap(Array, rhs_values_ptr, system_matrix.parent.n), "\n"))
    # end

    dim = system_matrix.parent.m # Matrix is quadratic, so we can use m or n

    rhs = unsafe_wrap(Array, rhs_values_ptr, dim)

    @debug "rhs = $rhs"
    @debug "lhs = $(unsafe_wrap(Array, lhs_values_ptr, dim))"

    # result = system_matrix \ rhs
    result = mna_solve(system_matrix, rhs)

    @debug "result = $result"

    for (index, value) in enumerate(result)
        unsafe_store!(lhs_values_ptr, 3.14, index)
    end
    return 0
end

Base.@ccallable function cleanup()::Cvoid
    mna_cleanup()
end

Base.@ccallable function log(log_string::Cstring)::Cvoid
    println("Log form Julia ExampleLib: $(unsafe_string(log_string))")
end

# function mat_ctojl(mat_ptr::Ptr{dpsim_csr_matrix})
#     mat = unsafe_load(mat_ptr)
#     return SparseMatrixCSR{0}(
#         mat_ptr.row_number, # Number of rows
#         mat_ptr.row_number, # Matrices are square
#         unsafe_wrap(Array, mat_ptr.rowIndex, mat_ptr.row_number+1), # Row pointers
#         unsafe_wrap(Array, mat_ptr.colIndex, mat_ptr.nnz), # Column indices
#         unsafe_wrap(Array, mat_ptr.values, mat_ptr.nnz) # Non-zero values
# )
# end

end # module
