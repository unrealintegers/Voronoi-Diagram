# Python visualisation for edge splits
# 
# For visualisation, print edges and watchtowers in the format of 
#   @E<edgeNum> <faceNum> <startX> <startY> <endX> <endY>
#   @W<faceNum> <X> <Y>
# 
# and pipe output to python script.
# 
# Setting edgeNum/faceNum to -1 disables colouring.

import sys
import matplotlib.pyplot as plt
import matplotlib

sys.stdin.reconfigure(encoding='utf-8-sig')
colors = ['orange', 'gold', 'lime', 'cyan', 'blue', 'indigo', 'violet']
matplotlib.use('qt5agg')

def getcolor(x):
    return 'black' if x == -1 else colors[x % len(colors)]

for line in sys.stdin:
    if line[0] != '@':
        print(line, end='')
    elif line[1] == 'W':
        n, *coord = line[2:].split()
        n, (x, y) = int(n), map(float, coord)

        plt.plot(x, y, marker='.', mfc=getcolor(n), mew=0, alpha=1, label=n, ms=12)
        
    elif line[1] == 'E':
        f, *coord = line[2:].split()
        f, (x1, y1, x2, y2) = int(f), map(float, coord)
        dx, dy = x2-x1, y2-y1
        
        plt.plot(x1, y1, 'ko', alpha=0.5, ms=3)
        plt.plot(x2, y2, 'ko', alpha=0.5, ms=3)
        plt.arrow(x1, y1, dx, dy, length_includes_head=True, width=0.08, fc=getcolor(f), 
                  shape='left', label='1', alpha=0.7, ec=None)

name = sys.argv[1]
plt.title(f'''Part 3 {"Square" if "sq" in name else "irregular"} Dataset{name[3:]}''')
plt.savefig(f"{name}.jpeg")
plt.show()