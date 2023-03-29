using SparseArrays
using CUDA.CUSPARSE
using CUSOLVERRF
using LinearAlgebra

# Hardwareawareness
struct AbstractAccelerator end
struct CUDAccelerator end

function systemCheck()
    accelerator = AbstractAccelerator()
    if has_cuda()
        @debug "CUDA available! Try using CUDA accelerator..."
        try
            CuArray(ones(1))
            # push!(accelerators, CUDAccelerator())
            accelerator = CUDAccelerator()
            @warn "CUDA driver available and CuArrays package loaded. Using CUDA accelerator..."
        catch e
            @warn "CUDA driver available but could not load CuArrays package."
        end
    elseif isempty(accelerator)
        @warn "No accelerator found."
    end

    return accelerator
end

# Housekeeping
function mna_init()
end

function mna_cleanup()
end

# Solving Logic
function mna_decomp(sparse_mat, accelerator::AbstractAccelerator)
    lu_decomp = SparseArrays.lu(sparse_mat)
    return lu_decomp
end

function mna_decomp(sparse_mat, accelerator::CUDAccelerator)
    matrix = CuSparseMatrixCSR(CuArray(sparse_mat)) # Sparse GPU implementation
    lu_decomp = CUSOLVERRF.RFLU(matrix; symbolic=:RF)
    return lu_decomp
end

function mna_solve(system_matrix, rhs, accelerator::AbstractAccelerator)
    return system_matrix \ rhs
end

function mna_solve(system_matrix, rhs, accelerator::CUDAccelerator)
    rhs_d = CuVector(rhs)
    ldiv!(system_matrix, rhs_d)
    return Array(rhs_d)
end