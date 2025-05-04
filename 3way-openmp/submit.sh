#!/bin/bash
#SBATCH --job-name=openmp_perf
#SBATCH --nodes=1
#SBATCH --ntasks=1                # Only need 1 MPI task for OpenMP
#SBATCH --cpus-per-task=20        # This is where you specify the max number of threads
#SBATCH --mem=16G
#SBATCH --time=08:00:00           # Allow up to 8 hours for all tests
#SBATCH --partition=ksu-gen.q,killable.q
#SBATCH --constraint=mole         # Restricting to mole class nodes as specified
#SBATCH --output=openmp_perf_%j.out
#SBATCH --error=openmp_perf_%j.err

# Load required modules
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a

echo "Running on host: $(hostname)"
echo "Starting at: $(date)"
echo "SLURM_JOB_ID: $SLURM_JOB_ID"
echo "SLURM_CPUS_PER_TASK: $SLURM_CPUS_PER_TASK"

# Export variable to control number of threads
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "OMP_NUM_THREADS: $OMP_NUM_THREADS"

# Set thread affinity for better performance
export OMP_PROC_BIND=close
export OMP_PLACES=cores
echo "Thread affinity settings: OMP_PROC_BIND=close, OMP_PLACES=cores"

# Make the performance test script executable
chmod +x performance_test.sh

# Run the performance tests
./performance_test.sh

echo "Performance testing finished at: $(date)"