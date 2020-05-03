#!/usr/bin/env python3

import sys
import os
import subprocess
import pprint
import operator
import itertools
import re

from plot import plot, plot_input

# --------------------------------- ADD YOUR BENCHMARKS HERE

STD_REPS = "10"

# --- Available functions for CLI
# List all function version choices for a benchmark that you want to execute here.
xyz_f = ["xyz_f1", "xyz_f2"]
for index, item in enumerate(xyz_f):
   xyz_f[index] = "xyz_f="+item

dotProduct = ["ref","auto1","auto2","vec1","2acc"]
for index, item in enumerate(dotProduct):
   dotProduct[index] = "dotProduct="+item


# --- Benchmarks
'''
    id:            unique benchmark id, used for COMPARE
    name:          name of the file to benchmark
    config:        list of benchmark configs containing:

         fun_configs:   CLI options that determine the function to be selected
                        format: funListName -> Definded above
         run_configs:   CLI options that determine different benchmark configurations to be executed
                        format: ["opt-char0=opt-value0", "opt-char1=opt-value1", ...]
         input_configs: CLI options that determine different inputs for functions
                        format: ("option-char", ["option-val0", "option-val1", ...])
	     			                or the following format:
                                ("option-char", ["i", operator, increment, lower_bound, upper_bound].
                                In this case, there is a loop that iterates over the desired range
						     Only one option-char is available
         xlabel:        Labels the x-axis for benchmarks that have an input config
                        format: same as one element of the input config, i.e.
                                ("option-char", ["option-val0-printable, ..., option-valn-printable"])
                        note that length of list must match the one of the corresponding input config
                        we match by index!
'''
BENCHMARKS = [
   {"id": 0,
    "name": "benchmark_test_macro",
    "config": [
       {
          "fun_configs": [],
          "run_configs": ["r="+STD_REPS],
          "input_configs": []
       },
    ]},
   {"id": 1,
    "name": "benchmark_test_xyz_f",
    "config": [
       {
          "fun_configs": xyz_f,
          "run_configs": ["r="+STD_REPS],
          "input_configs": []
       }
    ]},
   {"id": 2,
    "name": "benchmark_polyvest",
    "config": [
       {
          "fun_configs": [],
          "run_configs": ["r="+STD_REPS],
          "input_configs": []
       }
    ]},
   {"id": 3,
    "name": "benchmark_dotProduct",
    "config": [
       {
          "fun_configs": dotProduct,
          "run_configs": ["r=100000"],
          "input_configs": [("n", [2**i for i in range(0,7)])],
          "xlabel": ("n", [str(2**i) for i in range(0,7)])
       }
    ]},
   {"id": 4,
    "name": "benchmark_intersection",
    "config": [
       {
          "fun_configs": ["Polytope_intersectCoord=cached_ref", "Polytope_intersectCoord=ref"],
          "run_configs": ["intersect=intersectCoord,polytopeTranspose=false"],
          "input_configs": [("generator", intersectbodies)],
          "xlabel": [("generator", intersectdims)]
       },
       {
          "fun_configs": ["PolytopeT_intersectCoord=cached_nc1", "PolytopeT_intersectCoord=ref", "PolytopeT_intersectCoord=ref"],
          "run_configs": ["intersect=intersectCoord,polytopeTranspose=true"],
          "input_configs": [("generator", intersectbodies)],
          "xlabel": [("generator", intersectdims)]
       },
       {
          "fun_configs": [],
          "run_configs": ["intersect=intersect,polytopeTranspose=false", "intersect=intersect,polytopeTranspose=true"],
          "input_configs": [("generator", intersectbodies)],
          "xlabel": [("generator", intersectdims)]
       }
    ]} 
]

# --- Functions that should be compared

COMPARE = [2]


# ---------------------------- Parse python args
DO_BENCHMARKS = BENCHMARKS;
if(len(sys.argv)>1):
   DO_BENCHMARKS = [];
   
   for i in range(1,len(sys.argv)):
      benchmark = sys.argv[i]
      upd = list(filter(lambda b: b["name"] == benchmark, BENCHMARKS))
      DO_BENCHMARKS += upd
      if len(upd) == 0:
         print("ERROR: benchmark '{}' is not available!".format(benchmark));
         print("  list of available benchmarks:");
         for b in BENCHMARKS:
            print("     ",b);
         sys.exit(1);


         
"""
config contains the following:
fun_config :: [string]
run_config :: [string]
input_conifg :: [(char, [(readable, char)])]

from this we create the following list of config strings
'-f "<fun>" -n "<run,c=str(cconf)>"' 
for 
   fun : fun_config, 
   run : run_config,
   (c, conf) : input_config
   cconf : conf
"""
def get_config(config):
   funs = ['-f "{}"'.format(fun) for fun in config["fun_configs"]] or [""]
   inputs = ['-n "' + ','.join(c_prod) + '"' for c_prod in
             itertools.product(
                *(list(map(
                      lambda c: ['{}={}'.format(c[0], conf) for conf in c[1]],
                      config["input_configs"]
                )) + [config["run_configs"]])
             )
   ] or [""]
   return itertools.product(funs, inputs)


"""
benchmark contains 
config :: [dict]

we extract all config strings from the elements of this list and concat them
"""
def get_configs(benchmark):
   return [conf_string for conf_string in get_config(conf) for conf in benchmark["config"]]


"""
get x-label from benchmark_string by:
- matching on the option tag
- getting the value
- looking up the label of the value in the benchmark

this is ugly, but should work if we only choose labels on input_config...  
"""
def get_label(benchmark, benchmark_string):
   # all assertions needed for valid use of this function
   assert("xlabel" in benchmark and
          "input_configs" in benchmark and
          len(benchmark["xlabel"]) > 1 and
          isinstance(benchmark["xlabel"][0], str) and
          len(benchmark["xlabel"]) == len(benchmark["input_configs"])
   )
   pattern = '\-n\s*".*{}=([^,"]*)'.format(benchmark["labelx"][0])
   strval = re.search(pattern, benchmark_string).group(1)
   index = list(map(str, benchmark["input_configs"])).index(strval)
   return benchmark["labelx"][2][index]
   


"""
expects a list of strings as created by get_configs
this allows concatenating benchmarks -> get_configs creates product of config options, concatenating benchmark strings creates sum of config options
"""
def run_benchmark(bid, bname, config_strings):
   results = []

   for runconf,inputconf in config_strings:
      config_string = runconf + inputconf
      config_string_printable = (
         config_string
         .replace(" ", "_")
         .replace(",", "")
      ).replace("__","")

      print("# Running Benchmark '{}' with config '{}'...".format(bname, config_string));
      myenv = os.environ;
      proc = subprocess.Popen(
         [sys.path[0]+"/"+bname, runconf, inputconf],
         stdout=subprocess.PIPE,
         stderr=subprocess.PIPE,
         env = myenv
      );
      f = open(sys.path[0]+ "/out/" + bname + config_string_printable + ".out", "w")
      for line in proc.stdout:
         try:
            dict = eval(line)
            results.append((config_string_printable, dict))
            f.write(str(dict)+'\n')
         except:
            f.write(line.decode('utf-8'))
      f.close()

   return results


# create results dictionary containing merged benchmarks
compare_results = []

for benchmark in DO_BENCHMARKS:
   bname = benchmark["name"]
   result = run_benchmark(benchmark["id"],
                          bname,
                          get_configs(benchmark)
   )
   # get x-axis labels and add them to data
   if "xlabel" in benchmark: 
      result = list(map(lambda res: (*res, get_label(benchmark, res[0])), result))
      plot_input(sys.path[0], bname, result, benchmark["xlabel"][0])
   else:
      plot(sys.path[0], bname, result)

   if benchmark["id"] in COMPARE:
      compare_results.extend(result)
      

do_plot = True
plot_name = "benchmark_comparison"

if do_plot and compare_results:
   plot(sys.path[0], plot_name, compare_results)
