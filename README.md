# benchmarks
test des machines sous macos (eventuellement sur linux ou raspberry) du CPU (processeur en mono et multi threads) et du GPU
pour comparaison des temps d'execution.

également les commandes à lancer :  
  - hashcat -m 17600 -b (pour tester les temps d'execution avec OpenCL 1.2 ou Metal)
  - sysbench cpu --cpu-max-prime=20000 --threads=$NUMCPUS --time=0 --events=10000 run
  - calcul PI sur 10 000 (time echo "scale=10000; 4*a(1)" | bc -l)
  - tri V6 50 000 16 2 avec le programme de tri version 6 du bubble sort avec une suite aléatoire 
  - hardinfo

../..    
pour linux ubuntu , installer les paquets suivants : 
sudo apt install clinfo ocl-icd-opencl-dev build-essential libc6-dev 


pour carte nvidia : sudo apt install nvidia-opencl-dev ocl-icd-libopencl1 nvidia-opencl-dev 

pour carte AMD : sudo apt install opencl-amdgpu-pro ocl-icd-libopencl1 opencl-amdgpu-pro

pour intel avec gpu intégré : sudo apt install intel-opencl-icd ocl-icd-libopencl1 intel-opencl-icd
../..


pour le programme : 2-benchmark_multicoeurs.c
il utilise la variable d'environnement NUMPLUS pour determiner le nombre de threads
pour linux dans .profile  : 
export NUMCPUS=`grep -c '^processor' /proc/cpuinfo`
alias make='nice make -j$NUMCPUS --load-average=$NUMCPUS'

pour osx : 
export NUMCPUS=`sysctl -n hw.ncpu`
alias make='nice make -j$NUMCPUS --load-average=$NUMCPUS'




