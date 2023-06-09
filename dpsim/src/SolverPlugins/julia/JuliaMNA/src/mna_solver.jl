using SparseArrays
using Krylov            #solver for 32bit-version
using LinearOperators   #operator for solver
using CUDA              #for debugging
using CUDA.CUSPARSE
using CUDA.CUSOLVER     #defines zfd()
using CUSOLVERRF
using LinearAlgebra

# Hardwareawareness
struct AbstractAccelerator end
struct CUDAccelerator64bit end
struct CUDAccelerator32bit end

function find_accelerator()
    # CUDA Accelerator
    if has_cuda()
        @debug "CUDA available! Try using CUDA accelerator..."
        try
            CuArray(ones(1))
            # push!(accelerators, CUDAccelerator())
            @warn "CUDA driver available and CuArrays package loaded. Using CUDA accelerator..."
            try 
                GPU32bit = ENV["JL_GPU32BIT"]
                if GPU32bit == "true"
                    accelerator = CUDAccelerator32bit()
                    @warn "32Bit enabled on CUDA driver. Using 32bit variables on CUDA accelerator..."
                end
            catch e
                if e isa KeyError 
                    # If the variable does not exits, assume 64bit
                    @warn "Using 64bit variables on CUDA accelerator..."
                    accelerator = CUDAccelerator64bit()
                else
                    rethrow(e)
                end
            end
        catch e
            @warn "CUDA driver available but could not load CuArrays package."
            accelerator = AbstractAccelerator()
        end
    elseif !@isdefined accelerator
        @warn "No accelerator found."
        accelerator = AbstractAccelerator()
    end
    return accelerator
end

function systemcheck()
    try
        hwAwarenessDisabled = ENV["JL_MNA_DISABLE_AWARENESS"]
        if hwAwarenessDisabled == "false"
            return find_accelerator()
        end
    catch e
        if e isa KeyError
            # If variable does not exist, assume: hardware awareness enabled
            return find_accelerator()
        else
            rethrow(e)
        end
    end
    @info "Hardware awareness disabled... Using Fallback implementation"
    return AbstractAccelerator()
end

function GPU_32Bit_check(accelerator)
    if typeof(accelerator) == AbstractAccelerator
        @info "CUDA accelerator not available. Cannot define bit-length."
        return false
    end

    try
        GPU32bit = ENV["JL_GPU32BIT"]
        if GPU32bit == "true"
            @info "Using 32Bit variables on CUDA accelerator."
            return true
        end
    catch e
        if e isa KeyError 
            # If the variable does not exits, assume 64bit
            @info "Using 64Bit variables on CUDA accelerator."
            return false
        else
            rethrow(e)
        end
    end
    @info "Using 64Bit variables on CUDA accelerator."
    return false
end

# Housekeeping
function mna_init()
    global accelerator = systemcheck()
end

function mna_cleanup()
end

# Solving Logic
function mna_decomp(sparse_mat, accelerator::AbstractAccelerator)
    lu_decomp = SparseArrays.lu(sparse_mat)
    @info "CPU"
    return lu_decomp
end

function mna_decomp(sparse_mat, accelerator::CUDAccelerator64bit)
    matrix = CuSparseMatrixCSR(CuArray(sparse_mat))
    
    #CUSOLVERRF
    lu_decomp = CUSOLVERRF.RFLU(matrix; symbolic=:RF)

    @info "GPU"
    @debug "lu_decomp = $lu_decomp | $(typeof(lu_decomp))"
    return lu_decomp
end

function mna_decomp(sparse_mat, accelerator::CUDAccelerator32bit)
    #convert sparse_mat into 32bit version
    sparse_mat = SparseMatrixCSR{0}(
        sparse_mat.m, # Number of rows
        sparse_mat.n, # Matrices are square
        sparse_mat.rowptr, # Row pointers
        sparse_mat.colval, # Column indices
        convert.(Float32, sparse_mat.nzval) # Non-zero values
    )

    #To avoid zero-pivot, calculate permutation. For this, Sparse_CSC format is necessary
    sparse_csc = sparse(sparse_mat)
    global p = zfd(sparse_csc, 'O') #permutation later required to reverse it
    p .+=1
    sparse_csc = sparse_csc[:,p]    #apply permutation

    #calculate incomplete LU factorization on GPU
    global matrix = CuSparseMatrixCSR(CuArray(sparse_csc))  #required later for Krylov-solver
    lu_decomp = ilu02(matrix, 'O')

    @info "GPU"
    @debug "lu_decomp = $lu_decomp | $(typeof(lu_decomp))"
    return lu_decomp
end
mna_decomp(sparse_mat) = mna_decomp(sparse_mat, accelerator)


function mna_solve(system_matrix, rhs, accelerator::AbstractAccelerator)
    return system_matrix \ rhs
end

function mna_solve(system_matrix, rhs, accelerator::CUDAccelerator64bit)
    rhs_d = CuVector(rhs)
        
    #CUSOLVERRF
    ldiv!(system_matrix, rhs_d)
        
    @debug "res = $rhs_d"
    return Array(rhs_d)
end

# Solve Py = b with P as result of LU
function ldiv_ilu0!(P::CuSparseMatrixCSR, b, y)
    ldiv!(UnitLowerTriangular(P), b)
    ldiv!(y, UpperTriangular(P), b)
    return y
end

function mna_solve(system_matrix, rhs, accelerator::CUDAccelerator32bit)
    rhs_d = CuVector{Float32}(rhs)
    CUDA.@allowscalar @debug "rhs_d = $rhs_d"

    #create Operator that models P⁻¹ for the Krylov-solver
    n = length(rhs_d)
    T = eltype(rhs_d)
    symmetric = hermitian = false
    opM = LinearOperator(T, n, n, symmetric, hermitian, (y, b) -> ldiv_ilu0!(system_matrix, b, y))

    #use iterative Krylov-solver for solving
    x, stats = dqgmres(matrix, rhs_d, M=opM)    #bicgstab (no sol for small matrix), gmres or dqgmres?

    invp = invperm(p)   #calculate inverse permutation
    rhs_d = x[invp]     #apply inverse permutation on solution
    rhs_d = Array(rhs_d)
    rhs_d = Float64.(rhs_d) #convert back to 64bit
    
    @debug "lhs = $rhs_d, $(typeof(rhs_d))"
    return rhs_d
end
mna_solve(system_matrix, rhs) = mna_solve(system_matrix, rhs, accelerator)