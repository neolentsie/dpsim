using JuliaMNA


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

# # Read system matrix from file
# # system_matrix_strings = readlines("test/system_matrix_small.txt")
# system_matrix_strings = readlines("../system_matrix.txt")

# # Sanize strings
# system_matrix_strings = replace.(system_matrix_strings, r"[\[\],]" => "")

# # Convert system to dpsim_csr_matrix
# values = parse.(Float64, split(system_matrix_strings[1]))
# rowIndex = parse.(Cint, split(system_matrix_strings[2]))
# colIndex = parse.(Cint, split(system_matrix_strings[3]))

# system_matrix = dpsim_csr_matrix(
#     Base.unsafe_convert(Ptr{Cdouble}, values),
#     Base.unsafe_convert(Ptr{Cint}, rowIndex),
#     Base.unsafe_convert(Ptr{Cint}, colIndex),
#     parse(Int32, system_matrix_strings[4]),
#     parse(Int32, system_matrix_strings[5])
# )

# # Read right hand side vector from file
# # rhs_vector_strings = readlines("test/rhs_small.txt")
# rhs_vector_strings = readlines("../rhs.txt")

# # Sanize rhs strings and parse into Float64 vector
# rhs_vector_strings = replace.(rhs_vector_strings, r"[\[\],]" => "")
# rhs_vector = parse.(Float64, split(rhs_vector_strings[1]))

# Inialize left hand side vector

system_matrix = read_input(ArrayPath("test/system_matrix.txt"))
system_matrix_ptr = pointer_from_objref(system_matrix)

rhs_vector = read_input(VectorPath("test/rhs.txt"))

lhs_vector = zeros(Float64, length(rhs_vector))

init(Base.unsafe_convert(Ptr{dpsim_csr_matrix}, system_matrix_ptr))
@time solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))