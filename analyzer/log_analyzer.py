#%%
print("Hello world")
import os
import re
from matplotlib import pyplot

#%%
timestamps = []
values = []

filepath = os.path.join(os.path.dirname(__file__),"../log/logfile.txt")
print("go")
with open(filepath) as file: # Use file to refer to the file object
    #data = file.read()
    for line in file:
        prog = re.compile(r'(\d{10}.\d{6}) from G (-?\d.?\d{0,6}e?-?\d?\d?)')
        result = prog.match(line)
        #print(line)
        #print(result)
        if result:
            timestamps.append(float(result.groups()[0]))
            values.append(float(result.groups()[1]))
            for group in result.groups():
                print(group)

print(timestamps)
print(values)



# %%
pyplot.plot(timestamps, values)
pyplot.show()

# %%