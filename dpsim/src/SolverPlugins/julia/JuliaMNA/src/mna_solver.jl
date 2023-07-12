using SparseArrays
using CUDA              #for debugging
using CUDA.CUSPARSE
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

# own LU-format required for VERSION 3 of decomp
import Base: size
struct LU_comps{A<:CUDA.CUSPARSE.CuSparseMatrixCSR, P<:CUDA.CUSPARSE.CuSparseMatrixCSR}
    mat::A      #matrix with L and U
    P::P        #permutation matrix
end
size(a::LU_comps, k...) = size(a.mat,k...)

# own LU-Umfpack-format for VERSION 4 of decomp
struct LU_UMFPACK_comps{Rs<:StridedVecOrMat, p<:StridedVecOrMat, A<:CUDA.CUSPARSE.CuSparseMatrixCSR, Q<:CUDA.CUSPARSE.CuSparseMatrixCSR}
    Rs::Rs          #scaling vector for rhs (on CPU)
    p::p            #right permutation vector for rhs (on CPU)
    mat::A          #matrix with L and U (for calc on GPU)
    Q::Q            #left permutation matrix for solution (for calc on GPU)
end
size(a::LU_UMFPACK_comps, k...) = size(a.mat, k...)

function mna_decomp(sparse_mat, accelerator::CUDAccelerator32bit)
    
    ### VERSION 1 ####
    # LU: on GPU without sparse matrices
    begin
        matrix = CuArray{Float32}(sparse_mat)
        # @debug "matrix: $(typeof(matrix)), $(matrix)"
        lu_decomp = lu(matrix) #result on GPU in Float32
    end
    ### END Version 1 ####

    ### VERSION 2 ####
    #LU: on CPU without sparse matrices
    #transfer: LU to GPU without Sparse matrices
    # begin
    #     lu_decomp = lu(Array(sparse_mat)) #on CPU; Array() prevents UMFPACK-format
    #     A = cu(lu_decomp.factors)
    #     p_d = CuArray{Int32}(lu_decomp.ipiv)
    #     lu_decomp = LU{Float32, typeof(A), typeof(p_d)}(A, p_d, lu_decomp.info)   #LU on GPU
    # end
    ### END VERSION 2 ####

    ### VERSION 3 ####
    #LU: on CPU without sparse matrices
    #transfer: own LU-type to GPU WITH sparse matrices
    # begin
    #     lu_decomp = lu(Array(sparse_mat)) #on CPU; Array() prevents UMFPACK-format
    #     A = CuSparseMatrixCSR{Float32}(sparse(lu_decomp.factors))
    #     P = CuSparseMatrixCSR{Float32}(sparse(lu_decomp.P))
    #     lu_decomp = LU_comps(A,P)
    # end
    ### END VERSION 3 ###
    
    ### VERSION 4 ####
    #LU: on CPU (sparse input -> UMFPACK on CPU)
    #transfer: own LU-type; scaling, permutation for rhs-vec on CPU, but matrix and permutation of sol. on GPU
    #matrix = CuSparseMatrixCSR(CuArray{Float32}(sparse_mat))    #DOES NOT HELP -> LU still on CPU
    # begin
    #     lu_decomp = lu(sparse(sparse_mat)) #on CPU but UMFPACK-Format
    #     A = CuSparseMatrixCSR{Float32}(lu_decomp.L + lu_decomp.U - I)   #one matrix containing L and U
    #     dim = length(lu_decomp.p)
    #     Q = CuSparseMatrixCSR{Float32}(sparse(Matrix{Float32}(I, dim, dim)[:, lu_decomp.q]))
    #     lu_decomp = LU_UMFPACK_comps(lu_decomp.Rs, lu_decomp.p, A, Q)
    # end
    ### END VERSION 4 ###

    ### Version 5 #### #TODO: create own RFLU function
    # matrix = CuSparseMatrixCSR(CuArray{Float32}(sparse_mat))
    # @CUDA.allowscalar @debug "matrix: $(typeof(matrix)), $(matrix)"
    # #CUSOLVERRF
    # lu_decomp = CUSOLVERRF.RFLU(matrix; symbolic=:RF)
    ### END Version 5 ###


    #@info "GPU"
    #@debug "L: $(typeof(lu_decomp.L)), U: $(typeof(lu_decomp.U))"
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

function mna_solve(system_matrix, rhs, accelerator::CUDAccelerator32bit)
    
    ### For VERSION 1, 2 and 3 of decomp ####
    begin
        rhs_d = CuVector{Float32}(rhs)
        ldiv!(system_matrix, rhs_d)
    end
    ### END ####

    ### For VERSION 4 of decomp ####
    #solve on GPU with sparse matrices and UMFPACK
    # begin
    #     #load on GPU
    #     rhs_d = CuVector{Float32}((system_matrix.Rs .*rhs)[system_matrix.p])    #scaled, permuted rhs-vec on GPU

    #     #solve
    #     ldiv!(UpperTriangular(system_matrix.mat), ldiv!(UnitLowerTriangular(system_matrix.mat), rhs_d))
    #     rhs_d = system_matrix.Q*rhs_d
    # end
    ### END ####

    ### For Version 5 of decomp ####

    ### END #####
    
    # transfer to CPU, convert to 64bit
    rhs_d = Array(rhs_d)
    rhs_d = Float64.(rhs_d)
    # @debug "lhs = $rhs_d, $(typeof(rhs_d))"

    return rhs_d
end
mna_solve(system_matrix, rhs) = mna_solve(system_matrix, rhs, accelerator)

#Required for Version 3
import LinearAlgebra: ldiv!
function ldiv!(A::LU_comps, B::StridedVecOrMat)
    ldiv!(UpperTriangular(A.mat), ldiv!(B, UnitLowerTriangular(A.mat), A.P*B))
end