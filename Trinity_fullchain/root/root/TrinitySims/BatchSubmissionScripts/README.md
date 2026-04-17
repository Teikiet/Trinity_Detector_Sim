# Trinity Shower Simulation Batch Chain (ISOG + CHASM + CIO + GrOptics)

This folder contains scripts to automate batch submission of the full Trinity simulation chain on Hive using SLURM.

## Overview

This pipeline consists of:

1. **ISOG (IStayOnGround)** simulation prepared locally.
2. **CHASM, CIOReader, GrOptics** simulation run on Hive in batch jobs using SLURM.

The final output log file will include information from both ISOG and the rest of the simulation chain.

---

## Step-by-Step Instructions

### 1. Set Up CHASM Environment on Hive

```bash
conda create -n chasm python=3.10 -y
conda activate chasm
conda install pip
python3 -m pip install -e /storage/hive/project/phy-otte/shared/analysistools/TrinitySims/CHASM_NuSpacesim
pip install numpy matplotlib scipy eventio astropy
````

> Replace `chasm` with your preferred environment name if needed.

---

### 2. Prepare ISOG Simulation Locally

Run on your local machine where IFHB/ISOG is compiled:

* Edit the **pilot file** (e.g., `ISOG.pilot`) to set:

```plaintext
NSHOWERS 1000
```

* Modify **emergence and azimuth angle ranges** in `IStayOnGround.cpp`:

```cpp
double showerEM = TMath::DegToRad() * rand->Uniform(0, 15); // emergence angle: 0–15 deg
double showerZN = TMath::Pi()/2 - showerEM;                  // zenith
double showerAZ = TMath::DegToRad() * rand->Uniform(0, 15);  // azimuth: 0–15 deg
if (rand->Uniform(1) >= 0.5) {
    showerAZ = 2 * TMath::Pi() - showerAZ; // random sign
}
```

* Recompile ISOG after making these changes.

---

### 3. Run ISOG Simulations

Use the wrapper script locally:

```bash
./Prepare_TrinityISOGSimulations.sh
```

This will:

* Create timestamped simulation folders (e.g., `20250621_134501`) under the output directory.
* Run ISOG simulation locally.
* Save logs to `out_<timestamp>.txt` in each folder.

Make sure there is a `sleep 1` in the script to avoid timestamp collisions.

---

### 4. Copy Simulation Folders to Hive

Transfer the entire output from your local machine to the desired Hive directory, e.g.:

```
scp -r \\wsl.localhost\Ubuntu-22.04\root\TrinitySims\output\* <username>@login-hive.pace.gatech.edu:/storage/hive/project/phy-otte/<username>/TrinitySims/output```

This should contain:

* `yyyymmdd_hhmmss/ISOG/` with input/output files
* `out_ISOG_<timestamp>.txt` with logs

---

### 5. Submit Trinity Chain on Hive

Run this on Hive:

```bash
./Submit_TrinityShowersMCChainJobs_Slurm.sh (located in /storage/hive/project/phy-otte/shared/analysistools/TrinitySims/BatchSubmissionScripts/Hive_SequentialChainJobPerShower/)
```

This will:

* Loop over each simulation folder under the output directory
* Submit a SLURM job using:

```bash
runTrinityShowersMCSequentialChain.slurm.sh <yyyymmdd_hhmmss>
```

Each job:

* Runs on 10 cores
* Allocated for 15 hours
* Appends output and logs to `out_<timestamp>.txt` inside the same simulation folder

> This will simulate \~10⁵ showers in parallel up to GrOptics stage.

---

## Notes

* Paths in the scripts are currently hard-coded for `mfedkevych3`. Make sure to update them for your user if different.
* If your Python environment has a different name than `chasm`, update the activation line in `executeCHASM.sh` or elsewhere.

---


## Contact

For issues or questions, contact `mfedkevych3@gatech.edu`.

```

---
