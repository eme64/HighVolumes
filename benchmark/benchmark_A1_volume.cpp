#include "benchmark.hpp"
#include "../src/volume/volume_helper.hpp"


class Benchmark_A1 : public Benchmark_base {
    public:
        Benchmark_A1(std::string name, int reps, bool convergence, int warmup_reps, const std::string &generator, bool polytopeTranspose, const double time_ci_alpha_, const double results_ci_alpha_)
		: Benchmark_base(name, reps, convergence, warmup_reps, time_ci_alpha_, results_ci_alpha_), generator(generator), polytopeTranspose(polytopeTranspose) {}

    protected:
        void initialize () {
            std::cout << "initializing A1 data..." << std::endl;
            
            solved_body = solved_body_generator()->get(generator,polytopeTranspose);
	    solved_body->print();
	    assert(solved_body->is_normalized);
	    r0 = 1.0;
	    r1 = 2*solved_body->n;
	}
        void reset () {
            // nothing to reset
	}
        double run () {
	    FT res = volume(solved_body->n, r0, r1, solved_body->bcount, (const void**)solved_body->body, (const Body_T**)solved_body->type);
	    FT exact = solved_body->volume;
	    return (res - exact)/exact;
	}
	void finalize() {
	    pc_stack().reset();
            {
               PC_Frame<volume_cost_f> frame((void*)volume);
               frame.costf()(solved_body->n, solved_body->bcount, (const void**)solved_body->body, (const Body_T**)solved_body->type);
            }
            pc_stack().print();
	    pc_flops = pc_stack().flops();
	    pc_bytes = pc_stack().bytes();
	}
    private:
	const std::string generator;
	Solved_Body* solved_body;
	FT r0,r1;
	bool polytopeTranspose = false;
};

int main(int argc, char *argv[]){
    CLI cli(argc,argv,"benchmark");
    CLIFunctionsVolume cliFun(cli);
    
    int r = 100;
    int warmup = 0;
    double time_ci_alpha;
    double results_ci_alpha;
    cliFun.claimOpt('b',"Benchmarking configuration");
    cliFun.add(new CLIF_OptionNumber<int>(&r,'b',"r","100", 1, 100000));
    cliFun.add(new CLIF_OptionNumber<int>(&warmup,'b',"warmup","0", 0, 100000));
    cliFun.add(new CLIF_OptionNumber<double>(&time_ci_alpha,'b',"time_ci_alpha","0.95", 0, 1));
    cliFun.add(new CLIF_OptionNumber<double>(&results_ci_alpha,'b',"results_ci_alpha","0.95", 0, 1));

    std::string generator = "cube";
    auto &gen_map = solved_body_generator()->gen_map();
    cliFun.add(new CLIF_Option<std::string>(&generator,'b',"generator","cube_r1.0_10", gen_map));
    
    bool polytopeTranspose = false;
    cliFun.add(new CLIF_Option<bool>(&polytopeTranspose,'b',"polytopeTranspose","false", {
                                                     {"false",{false, "Polytope format / rows"}},
						     {"true",{true, "PolytopeT format / columns"}} }));

    cliFun.preParse();
    if (!cli.parse()) {return -1;}
    cliFun.postParse();

    Benchmark_A1 b("A1_volume", r, true, warmup, generator, polytopeTranspose, time_ci_alpha, results_ci_alpha);
    b.run_benchmark();
}
