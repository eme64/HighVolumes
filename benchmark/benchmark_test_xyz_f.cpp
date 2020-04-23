#include "benchmark.hpp"

#include "../src/volume/volume_helper.hpp"


class Test_xyz_f : public Benchmark_base {
    public:
        Test_xyz_f(std::string name, int reps, bool convergence, int warmup_reps) : Benchmark_base(name, reps, convergence, warmup_reps) {}

        void initialize () {
            box = Polytope_new_box(4,2);
        }
        void reset () {
            // Nothing to reset
        }
        double run () {
            xyz_f(box,0.1,4);
        }

    private:
        Polytope* box; 
        xyz_f_t selected;
};

int main(int argc, char *argv[]){
    CLI cli(argc,argv,"benchmark");
    CLIFunctionsVolume cliFun(cli);
    
    cli.addOption('r', "100", "number of repetitions");
  
    cliFun.preParse();
    if (!cli.parse()) {return -1;}
    cliFun.postParse();

    int reps = std::stoi(cli.option('r'));

    Test_xyz_f *benchmark = new Test_xyz_f("test_xyz_f", reps, false, 0);
    benchmark->run_benchmark();
}
