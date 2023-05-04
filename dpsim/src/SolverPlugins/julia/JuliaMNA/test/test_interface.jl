push!(LOAD_PATH, "/home/wege/Projects/dpsim-refactor/dpsim/src/SolverPlugins/julia/JuliaMNA")

using Pkg
Pkg.activate(".")
Pkg.status()
# Pkg.instantiate()

using JuliaMNA
using Profile

struct ArrayPath path::String end
struct VectorPath path::String end

function read_input(path::ArrayPath)
    # Read system matrix from file
    system_matrix_strings = readlines(path.path)

    # Sanize strings
    system_matrix_strings = replace.(system_matrix_strings, r"[\[\],]" => "")

    # Convert system to dpsim_csr_matrix
    values = parse.(Float64, split(system_matrix_strings[1]))
    rowIndex = parse.(Cint, split(system_matrix_strings[2]))
    colIndex = parse.(Cint, split(system_matrix_strings[3]))

    system_matrix = dpsim_csr_matrix(
        Base.unsafe_convert(Ptr{Cdouble}, values),
        Base.unsafe_convert(Ptr{Cint}, rowIndex),
        Base.unsafe_convert(Ptr{Cint}, colIndex),
        parse(Int32, system_matrix_strings[4]),
        parse(Int32, system_matrix_strings[5])
    )

    return system_matrix
end

function read_input(path::VectorPath)
    # Reard right hand side vector from file
    rhs_vector_strings = readlines(path.path)

    # Sanize rhs strings and parse into Float64 vector
    rhs_vector_strings = replace.(rhs_vector_strings, r"[\[\],]" => "")
    rhs_vector = parse.(Float64, split(rhs_vector_strings[1]))
end

GC.enable(false) # We cannot be sure that system_matrix is garbage collected before the pointer is passed... 
system_matrix = read_input(ArrayPath("$(@__DIR__)/system_matrix_small.txt"))
# system_matrix = read_input(ArrayPath("test/system_matrix_small.txt"))
system_matrix_ptr = pointer_from_objref(system_matrix)

# @show unsafe_wrap(Array, system_matrix.values, system_matrix.nnz)

rhs_vector = read_input(VectorPath("$(@__DIR__)/rhs_small.txt"))
# rhs_vector = read_input(VectorPath("test/rhs_small.txt"))

lhs_vector = zeros(Float64, length(rhs_vector))

init(Base.unsafe_convert(Ptr{dpsim_csr_matrix}, system_matrix_ptr))
GC.enable(true)

# @time solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))
solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))
# @profile solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))

## Profling
# @time begin
#     @profile begin
#         for i in 1:1000
#             solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))
#         end
#     end
# end
begin
    begin
        for i in 1:1000
            solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))
        end
    end
end

# using StatProfilerHTML
# statprofilehtml()


# using BenchmarkTools
# @benchmark begin
#     for i in 1:1000
#         solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))
#     end
# end
