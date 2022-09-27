module JuliaLib

export get_mna_plugin, example_init, example_decomp, example_solve, example_cleanup, example_log

# Julia representation of DPsim structs
mutable struct dpsim_csr_matrix
    values::Ptr{Cdouble}    #size: nnz
    rowIndex::Ptr{Cint}     #size: nnz
    colIndex::Ptr{Cint}     #size: row_number+1
    row_number::Cint        #number of rows of the matrix
    nnz::Cint               #number of non-zero elements in the matrix
end

# Function dummies
Base.@ccallable function example_init(matrix_ptr::Ptr{dpsim_csr_matrix})::Cint
    println("initialize Julia ExampleLib...")
    return 0
end

Base.@ccallable function example_decomp(matrix_ptr::Ptr{dpsim_csr_matrix})::Cint
    println("decomposition with Julia ExampleLIb...")
    return 0
end

Base.@ccallable function example_solve(rhs_values_ptr::Ptr{Cdouble}, lhs_values_ptr::Ptr{Cdouble})::Cint
    println("solve with Julia ExampleLib...")
    return 0
end

Base.@ccallable function example_cleanup()::Cvoid
    println("cleanup with Julia ExampleLib...")
end

Base.@ccallable function example_log(log_string::Cstring)::Cvoid
    println("Log form Julia ExampleLib: $(unsafe_string(log_string))")
end

end # module
