#!/bin/bash
#SBATCH --job-name=pthread_perf
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=20   # Maximum cores on a mole node
#SBATCH --mem=16G
#SBATCH --time=08:00:00      # Allow up to 8 hours for all tests
#SBATCH --partition=ksu-gen.q,killable.q
#SBATCH --constraint=mole    # Restricting to mole class nodes as specified
#SBATCH --output=perf_test_%j.out
#SBATCH --error=perf_test_%j.err

# Load required modules as specified in the project description
module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a

# Print job information
echo "Running on host: $(hostname)"
echo "Starting at: $(date)"
echo "SLURM_JOB_ID: $SLURM_JOB_ID"
echo "SLURM_CPUS_PER_TASK: $SLURM_CPUS_PER_TASK"

# Make the performance testing script executable
chmod +x performance_test.sh

# Run the performance testing script
./performance_test.sh

echo "Performance testing finished at: $(date)"