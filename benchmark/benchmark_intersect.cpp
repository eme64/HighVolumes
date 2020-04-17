#include "benchmark.hpp"
#include "../src/volume/volume_helper.hpp"


class Benchmark_intersect : public Benchmark_base {
    public:
        Benchmark_intersect(std::string name, int reps, bool convergence, int warmup_reps, int n, const std::string &generator) : Benchmark_base(name, reps, convergence, warmup_reps), n(n), generator(generator) {}

    protected:
        void initialize () {
            std::cout << "initializing intersect data..." << std::endl;
	    
	    x = (FT*)(aligned_alloc(32, n*sizeof(FT))); // align this to 32
	    d = (FT*)(aligned_alloc(32, n*sizeof(FT))); // align this to 32
            
	    if(generator.compare("cube") == 0) {
                body = Polytope_new_box(n,1);
		type = &Polytope_T;
	    }else if(generator.compare("sphere") == 0) {
		FT* center = (FT*)malloc(n*sizeof(FT));
		for(int i=0; i<n; i++) {center[i] = 0;}; center[0] = 1;
                body = Sphere_new(n,2,center);
		type = &Sphere_T;
	    } else {
	        std::cout << "Error: did not find generator " << generator << "\n";
		assert(false);
	    }
            
	    int cache_size = type->cacheAlloc(body);
            cache = aligned_alloc(32, cache_size); // align this to 32

	    reset();
        }
        void reset () {
            for(int i=0; i<n;i++) {
	        x[i] = prng_get_random_double_0_1()*0.1;
	        d[i] = prng_get_random_double_normal();
	    }
	    type->cacheReset(body, x, cache);
	    dd = prng_get_random_int_in_range(0,n-1);
	}
        double run () {
	    FT t0, t1;
	    type->intersect(body, x, d, &t0, &t1);
            return t0-t1;
	}
    protected:
	const std::string generator;
	int n;
	void* body;
	Body_T* type;
	FT* x;
	FT* d;
	int dd;
	void* cache;
};

class Benchmark_intersectCoord : public Benchmark_intersect {
    public:
        Benchmark_intersectCoord(std::string name, int reps, bool convergence, int warmup_reps, int n, const std::string &generator) : Benchmark_intersect(name, reps, convergence, warmup_reps, n, generator) {}
    protected:
        double run () {
	    FT t0, t1;
	    type->intersectCoord(body, x, dd, &t0, &t1, cache);
            return 0;
	}
};

int main(int argc, char *argv[]){
    CLI cli(argc,argv,"benchmark");
    CLIFunctionsVolume cliFun(cli);

    cli.addOption('r', "100", "number of repetitions");
    
    int n = 20;
    cliFun.claimOpt('b',"Benchmarking configuration");
    cliFun.add(new CLIF_OptionNumber<int>(&n,'b',"n","20", 1, 100));
    
    std::string generator = "cube";
    cliFun.add(new CLIF_Option<std::string>(&generator,'b',"generator","cube", std::map<std::string, std::string>{
                                                     {"cube","cube"},
						     {"sphere","sphere"} }));
    
    std::string intersect = "intersect";
    cliFun.add(new CLIF_Option<std::string>(&intersect,'b',"intersect","intersect", std::map<std::string, std::string>{
                                                     {"intersect","intersect"},
						     {"intersectCoord","intersectCoord"} }));

    cliFun.preParse();
    if (!cli.parse()) {return -1;}
    cliFun.postParse();

    int reps = std::stoi(cli.option('r'));
    
    if(intersect.compare("intersect")==0) {
        Benchmark_intersect b("intersect", reps, true, 0, n, generator);
        b.run_benchmark();
    } else {
        Benchmark_intersectCoord b("intersectCoord", reps, true, 0, n, generator);
        b.run_benchmark();
    }
}