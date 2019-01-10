import subprocess

input_file = "twitter_dataset_small_v2.csv"
configuration_file = "cluster.conf"
metric = "euclidean"
iter = 1
for j in range (iter):
	k = str(j)
	output_file = "output_"+k
	subprocess.call(["./cluster", "-i", input_file, "-c", configuration_file, "-o", output_file, "-d", metric])