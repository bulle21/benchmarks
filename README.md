# benchmarks
test des machines sous macos (eventuellement sur linux ou raspberry) du CPU (processeur en mono et multi threads) et du GPU
pour comparaison des temps d'execution.

également les commandes à lancer :  
  - hashcat -m 17600 -b (pour tester les temps d'execution avec OpenCL 1.2 ou Metal)
  - sysbench cpu --test=cpu --cpu-max-prime=20000 --num-threads=4 --time=0 --events=10000 run
  - calcul PI sur 10 000 (time echo "scale=10000; 4*a(1)" | bc -l)
  - tri V6 50 000 16 2 avec le programme de tri version 6 du bubble sort avec une suite aléatoire 
  - hardinfo
  - 
