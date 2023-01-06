using JuliaLib, Test
using Suppressor

@testset "JuliaLib" begin
    
    @test begin
        testfunc = @cfunction(JuliaLib.example_log, Cvoid, (Cstring,))
        @capture_out @ccall $(testfunc)("Hello Test!"::Cstring)::Cvoid
    end == "Log form Julia ExampleLib: Hello Test!\n"

    @test begin
        testfunc = @cfunction(JuliaLib.example_cleanup, Cvoid, ())
        @capture_out @ccall $(testfunc)()::Cvoid
        # @capture_out ccall(testfunc, Cvoid, ())
    end == "cleanup with Julia ExampleLib...\n"

    # testmatrix = JuliaLib.dpsim_csr_matrix(Ref(collect(Cdouble,1.0)), collect(Cint, 1), collect(Cint, 1), 1, 1)

    # @test begin
    #     testfunc = @cfunction(JuliaLib.example_decomp, Cint, (Ptr{JuliaLib.dpsim_csr_matrix},))
        
    # end
end