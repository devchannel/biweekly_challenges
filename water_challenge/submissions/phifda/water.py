import sys
import time

with open(sys.argv[1]) as file:
    heights = [int(i) for i in file.read().split(',')]

start = time.clock()

totalWater = 0
currentMaxHeight = 0
currentMaxHeightIndex = 0
currentFill = 0
count = 0

for n in range(len(heights)):
    if heights[n] >= currentMaxHeight:
        area = currentMaxHeight * (n - currentMaxHeightIndex)
        totalWater += area - currentFill
        currentFill = 0
        currentMaxHeightIndex = n
        currentMaxHeight = heights[n]
    currentFill += heights[n]


heights = list(reversed(heights[currentMaxHeightIndex:]))

currentMaxHeight = 0
currentMaxHeightIndex = 0
currentFill = 0

for n in range(len(heights)):
    if heights[n] >= currentMaxHeight:
        area = currentMaxHeight * (n - currentMaxHeightIndex)
        totalWater += area - currentFill
        currentFill = 0
        currentMaxHeightIndex = n
        currentMaxHeight = heights[n]
    currentFill += heights[n]

end = time.clock()
time = end - start

print(totalWater)
print("{0:.4f}".format(time * 1000))
