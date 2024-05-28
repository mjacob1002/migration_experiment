import linecache
# from matplotlib import pyplot as plt
RESULTS_DIR = "migration_results/"
MB = 1024 * 1024

filesizes = []
read1_times = []
read2_times = []

starting_size = 4
while starting_size < 2048:
    filesize = starting_size * MB
    filename = f"{RESULTS_DIR}{starting_size}MB.txt.results"
    with open(filename, "r") as f:
        readtime1_total = 0
        readtime2_total = 0
        counter = 0
        while True:
            line = f.readline()
            if not line or len(line) == 0:
                break
            counter += 1
            line = line.strip()
            numbers = line.split(",")
            readtime1, readtime2 = float(numbers[0]), float(numbers[1])
            readtime1_total += readtime1
            readtime2_total += readtime2
        readtime1_avg = readtime1_total / counter
        readtime2_avg = readtime2_total / counter
        filesizes.append(starting_size)
        read1_times.append(readtime1_avg)
        read2_times.append(readtime2_avg)
    starting_size *= 4

print(filesizes)
print(read1_times)
print(read2_times)

plt.plot(filesizes, read1_times, label="Max read time before migration", color="r")
plt.plot(filesizes, read2_times, label="Max read time after migration", color="b")
plt.xlabel("File size (MB)")
plt.ylabel("Time (seconds)")
plt.title("Read times of 2 readers with load balancing")
plt.savefig("migration_plot", format="png")

