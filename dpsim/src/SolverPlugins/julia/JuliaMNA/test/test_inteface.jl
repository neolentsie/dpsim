using JuliaMNA

# Read system matrix from file
system_matrix_strings = readlines("../system_matrix_small.txt")
# system_matrix_strings = readlines("../system_matrix.txt")

system_matrix_strings[1] = replace(system_matrix_strings[1], r"[\[\],]" => "")
system_matrix_strings[2] = replace(system_matrix_strings[2], r"[\[\],]" => "")
system_matrix_strings[3] = replace(system_matrix_strings[3], r"[\[\],]" => "")

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

# Read right hand side vector from file
rhs_vector_strings = readlines("../rhs_small.txt")
# rhs_vector_strings = readlines("../rhs.txt")

# Sanize rhs strings and parse into Float64 vector
rhs_vector_strings[1] = replace(rhs_vector_strings[1], r"[\[\],]" => "")
rhs_vector = parse.(Float64, split(rhs_vector_strings[1]))

# Inialize left hand side vector
lhs_vector = zeros(Float64, length(rhs_vector))

system_matrix_ptr = pointer_from_objref(system_matrix)

init(Base.unsafe_convert(Ptr{dpsim_csr_matrix}, system_matrix_ptr))
solve(Base.unsafe_convert(Ptr{Cdouble}, rhs_vector), Base.unsafe_convert(Ptr{Cdouble}, lhs_vector))