#!/bin/bash
#SBATCH --job-name=mpi_perf
#SBATCH --nodes=1
#SBATCH --ntasks=20               # Number of MPI processes
#SBATCH --mem=16G
#SBATCH --time=08:00:00			#Allow up to 8 hours for all tests
#SBATCH --partition=ksu-gen.q,killable.q
#SBATCH --constraint=mole		# Restricting to mole class nodes as specified
#SBATCH --output=mpi_perf_%j.out
#SBATCH --error=mpi_perf_%j.err

module load CMake/3.23.1-GCCcore-11.3.0 foss/2022a OpenMPI/4.1.4-GCC-11.3.0

echo "Running on host: $(hostname)"
echo "Starting at: $(date)"
echo "SLURM_JOB_ID: $SLURM_JOB_ID"
echo "SLURM_NTASKS: $SLURM_NTASKS"

chmod +x performance_test.sh
./performance_test.sh

echo "Performance testing finished at: $(date)"
