import os
MB = 1024 * 1024

starting_size = 4 # 4MB
num_buffer_chares = 2
num_readers = 2

while starting_size < 2048:
    filesize = starting_size * MB
    filename = f"{starting_size}MB.txt"
    with open(filename, "a") as f:
        num_bytes_written = 0
        while num_bytes_written < (filesize): # create the files to be used
            f.write("abcd")
            num_bytes_written += 4
    os.system(f"./charmrun +p4 iotest {num_buffer_chares} {filesize} {num_readers} {filename}") # run 3 trials at each file size
    os.system(f"./charmrun +p4 iotest {num_buffer_chares} {filesize} {num_readers} {filename}")
    os.system(f"./charmrun +p4 iotest {num_buffer_chares} {filesize} {num_readers} {filename}")
    starting_size *= 4 # multiply by 4 to start making the next file

