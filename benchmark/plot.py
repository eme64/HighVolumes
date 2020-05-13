import matplotlib.pyplot as plt
import numpy as np
import re


def plot(path, plot_name, dict_list, x_option, title, x_label, y_label):
    if plot_name == None:
        plot_name = "plot"
    plot_name = plot_name + "_"
   
    x_values = []
    x_flops = {}
    x_bytes = {}
    time_function_names = []
    time_function_heights = {}
    time_function_std_devs = {}
    for dict in dict_list:
        # cut out x_label option
        pattern = '{}=[^,]*'.format(x_option)
        name = ''.join(re.compile(pattern).split(dict[0]))
        time_dict = (dict[1])['time']
        ival = dict[2]
        time_fun_name = time_dict['name_t']+name
        time_fun_height = float(time_dict['mean'])
        time_fun_std_dev = float(time_dict['std_dev'])        
        performance_counters = (dict[1])['performance_counter']
        flops = performance_counters['flops']
        bytes = performance_counters['bytes']
        if not ival in x_values:
            x_values.append(ival)
        if time_fun_name not in time_function_names:
            time_function_names.append(time_fun_name)
            time_function_heights[time_fun_name] = [time_fun_height]
            time_function_std_devs[time_fun_name] = [time_fun_std_dev]
            x_flops[time_fun_name] = [flops]
        else:
            time_function_heights[time_fun_name].append(time_fun_height)
            time_function_std_devs[time_fun_name].append(time_fun_std_dev)
            x_flops[time_fun_name].append(flops)    

    # Runtime Plot
    plt.title(title[0])
    plt.xlabel(x_label[0])
    plt.ylabel(y_label[0])
	        
    x_ticks = []
    for val in x_values:
        x_ticks.append(int(val))
    plt.xticks(x_ticks)

    i = 0
    for name in time_function_names:
        #plt.bar(y_pos[i], time_function_heights[name], yerr=time_function_std_devs[name], ecolor='red', width=bar_width, label=name)
        plt.plot(x_ticks, time_function_heights[name], label=name)
        i += 1
	
    plt.ylim(bottom=0)
	
    plt.legend(bbox_to_anchor=(1, 1), loc="upper left")

    plt.savefig(path+"/plots/"+plot_name+"runtime_mean.png", bbox_inches = "tight")

    plt.clf()

    # Performance plot
    plt.title(title[1])
    plt.xlabel(x_label[1])
    plt.ylabel(y_label[1])

    plt.xticks(x_ticks)

    i = 0
    for name in time_function_names:
        time_function_performance = []
        for index, item in enumerate(time_function_heights[name]):
            time_function_performance.append(x_flops[name][index]/item)
        plt.plot(x_ticks, time_function_performance, label=name)
        i += 1
	
    plt.ylim(bottom=0)
	
    plt.legend(bbox_to_anchor=(1, 1), loc="upper left")

    plt.savefig(path+"/plots/"+plot_name+"performance_mean.png", bbox_inches = "tight")

    plt.clf()
