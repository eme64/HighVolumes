#include "benchmark.hpp"
#include "../src/volume/volume_helper.hpp"


class Benchmark_dotProduct : public Benchmark_base {
    public:
        Benchmark_dotProduct(std::string name, int reps, bool convergence, int warmup_reps, int n, const std::string &uv) : Benchmark_base(name, reps, convergence, warmup_reps), n(n), uv(uv) {}

    protected:
        void initialize () {
            std::cout << "initializing dotProduct data..." << std::endl;
            
	    u = (FT*)(aligned_alloc(32, n*sizeof(FT))); // align this to 32
	    v = u;
	    if(uv.compare("uv")) {
	       v = (FT*)(aligned_alloc(32, n*sizeof(FT))); // align this to 32
	    }
	    reset();
        }
        void reset () {
            for(int i=0; i<n; i++) {
	        u[i] = prng_get_random_double_0_1();
	        v[i] = prng_get_random_double_0_1();
	    }
	}
        double run () {
            dotProduct(u,v,n);
	    return 0;
	}
    public:
	void performance_count() {
	    pc_stack().reset();
            {
               PC_Frame<dotProduct_cost_f> frame((void*)dotProduct);
               frame.costf()(n);
            }
            pc_stack().print();
	}
    private:
	int n;
	FT* u;
	FT* v;
	const std::string uv;
};

int main(int argc, char *argv[]){
    CLI cli(argc,argv,"benchmark");
    CLIFunctionsVolume cliFun(cli);
    
    int n = 20;
    int r = 100;
    cliFun.claimOpt('b',"Benchmarking configuration");
    cliFun.add(new CLIF_OptionNumber<int>(&n,'b',"n","20", 1, 1000));
    cliFun.add(new CLIF_OptionNumber<int>(&r,'b',"r","100", 1, 100000000));
   
    std::string uv = "uv";
    cliFun.add(new CLIF_Option<std::string>(&uv,'b',"uv","uv", std::map<std::string, std::string>{
                                                     {"uv","uv"},
						     {"vv","vv"} }));

    cliFun.preParse();
    if (!cli.parse()) {return -1;}
    cliFun.postParse();

    Benchmark_dotProduct b("dotProduct", r, true, 0, n, uv);
    b.run_benchmark();
    b.performance_count();
}
