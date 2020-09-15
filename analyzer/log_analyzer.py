#%%
print("Hello world")
import os
import re
from matplotlib import pyplot, rc

#%%
timestamps = []
values = []

filepath = os.path.join(os.path.dirname(__file__),"../log/logfile.txt")
i=0
first_timestamp = 0
with open(filepath) as file: # Use file to refer to the file object
    for line in file:
        prog = re.compile(r'(\d{10}.\d{1,6}) from G (-?\d.?\d{0,6}e?-?\d?\d?)')
        result = prog.match(line)
        if result:
            if i == 0:
                i+=1
                first_timestamp = float(result.groups()[0])
            timestamps.append(float(result.groups()[0]) - first_timestamp)
            values.append(float(result.groups()[1]))
            for group in result.groups():
                print(group)

print(timestamps)
print(values)



# %%
rf = 10
dt = '0_'
fig = pyplot.figure()
pyplot.plot(timestamps, values)
pyplot.rcParams.update({'font.size': 22})
figpath = os.path.join(os.path.dirname(__file__),"../output/rf_{}_dt_{}.png".format(rf,dt))
pyplot.title('rf={}, dt={} [s]'.format(rf,dt))
pyplot.xlabel('seconds')
pyplot.xlim(0,1)
fig.savefig(figpath)
pyplot.show()

# %%