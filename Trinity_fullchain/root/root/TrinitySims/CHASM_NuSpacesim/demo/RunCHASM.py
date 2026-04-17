import numpy as np
import matplotlib.pyplot as plt
import CHASM as ch
import sys
import time
import argparse as ap


def main():
    # Only define optional flags; legacy positionals are parsed manually
    disableAbsorbtion = False
    parser = ap.ArgumentParser(
        description="Run CHASM with 12 legacy positional args and optional flags."
    )
    parser.add_argument('--seed', type=int, help='Random seed for reproducibility', default=0)
    parser.add_argument('--abstable', type=str, help='Atmosphere table to use', default='m5_new')

    # Parse known options; leave the rest (legacy positionals + any unknown flags)
    opt, leftover = parser.parse_known_args()

    if opt.seed != 0:
        print(f"Using provided seed: {opt.seed}")
        np.random.seed(opt.seed)
    else:
        print("No seed provided, using random seed.")

    abstable_name = opt.abstable  # MODTRAN atmosphere
    
    if abstable_name == "none":
        print("Disabling absorbtion")
        disableAbsorbtion = True
        abstable_name = "m5_new" # just set it to a value so that it plays nicely
                                # this doesnt affect anything since absorbtion is turned off. 


    startT = time.time()

    minLambda = 200 # nm
    maxLambda = 1000 # nm
    num_bins = 80 # 10nm per bin for high resolution with atmosphere

    # Treat all leftover tokens as legacy positionals (allow negatives)
    pos = leftover

    # Need at least 12 legacy positional args
    if len(pos) < 12:
        print("Usage: python RunCHASM.py theta phi telX telY telZ telRad xMax nMax x0 lam esh outFile [--seed N] [--abstable NAME]")
        sys.exit(1)

    # Ignore any extra positionals beyond the required 12
    if len(pos) > 12:
        ignored = pos[12:]
        print(f"Ignoring extra positional args: {ignored}")

    # Unknown flags would appear only if they start with '--'
    unknown_flags = [t for t in pos[12:] if t.startswith('--')]
    if unknown_flags:
        print(f"Ignoring unknown flags: {unknown_flags}")

    print("Running CHASM:\n")
    print("Will Simulate the following configuration:\n")

    # Legacy positional mapping (order unchanged)
    theta = float(pos[0])
    phi = float(pos[1])
    telX = float(pos[2])
    telY = float(pos[3])
    telZ = float(pos[4])

    telRad = float(pos[5])

    xMax = float(pos[6])
    nMax = float(pos[7])
    x0 = float(pos[8])
    lam = float(pos[9])

    esh = float(pos[10])

    outFile = pos[11]

    print("Polar angle of shower: {:.2e}\nPhi angle of shower: {:.2e}\nTelescope Position: X: {:.2e} Y: {:.2e} Z: {:.2e}\n".format(theta, phi, telX, telY, telZ))
    print("Radius of detector sphere: {:.2e}\nShower Properties: XMax: {:.2e} NMax: {:.2e} X0: {:.2e} Lambda: {:.2e}".format(telRad,xMax,nMax,x0,lam))
    print("Shower Energy: {:.2e}\n".format(esh))
    print("Using Abstable: {}\n".format(abstable_name))
    print("Wavelength Range: {} nm to {} nm with {} bins\n".format(minLambda, maxLambda, num_bins))

    print("Output File: {}".format(outFile))
    Nxc = 1
    Nyc = 1

    counter = np.empty([Nxc*Nyc,3])
    counter[:,0] = telX
    counter[:,1] = telY
    counter[:,2] = telZ

    # Set the abstable to use
    ch.useAbstable(abstable_name)

    sim = ch.ShowerSimulation()
    sim.add(ch.UpwardAxis(theta,phi,maximum_altitude=10000.,curved=True))
    sim.add(ch.GHShower(xMax,nMax,x0,lam))
    sim.add(ch.SphericalCounters(counter,telRad))
    sim.add(ch.Yield(minLambda,maxLambda,N_bins=num_bins))  # 10 nm per bin

    if disableAbsorbtion == True: 
        # if no absorbtion requested, disable it
        sig = sim.run(mesh=True,att=False)
    else:
        # otherwise, run as normal
        sig = sim.run(mesh=True,att=True)
    sig.set_shower_energy(esh)
    ch.write_ei_file(sig, outFile)

    print("--- Time Elapsed: {} s".format( time.time() - startT))

if __name__ == "__main__":
    main()