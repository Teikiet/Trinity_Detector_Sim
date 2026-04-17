import numpy as np
import matplotlib.pyplot as plt
import CHASM as ch
import sys
import time
import os
def main():
	startT = time.time()
	print("Running CHASM:\n")
	print("Will Simulate the following configuration:\n")
	telX = 150000
	telY = 0
	telZ = 3000
	theta = np.arctan(telX/telZ)
	phi = np.radians(0)

	telRad = 0.8
	
	xMax = 700
	nMax = 4e7
	x0 = 0
	lam = 150

	esh = 1e17
	
	outFile = "/home/jordan/simbox/Output/sampleShowers/dat/sampleShower_{:.3f}_{:.3f}_{:d}_{:d}_{:d}.dat".format(np.rad2deg(theta),np.rad2deg(phi),int(telX),int(telY),int(telZ))
	
	print("Polar angle of shower: {:.2e}\nPhi angle of shower: {:.2e}\nTelescope Position: X: {:.2e} Y: {:.2e} Z: {:.2e}\n".format(theta, phi, telX, telY, telZ))
	print("Radius of detector sphere: {:.2e}\nShower Properties: XMax: {:.2e} NMax: {:.2e} X0: {:.2e} Lambda: {:.2e}".format(telRad,xMax,nMax,x0,lam))
	print("Shower Energy: {:.2e}\n".format(esh))
	
	print("Output File: {}".format(outFile))

	counters = np.empty([1,3])
	r = np.sqrt(telX**2 + telY**2 + telZ**2)
	print("telescope at radius {:.2e}\n".format(r))
	counters[:,0] = telX
	counters[:,1] = telY
	counters[:,2] = telZ

	sim = ch.ShowerSimulation()
	sim.add(ch.UpwardAxis(theta,phi,maximum_altitude=telZ+1000,curved=True))
	sim.add(ch.GHShower(xMax,nMax,x0,lam))
	sim.add(ch.SphericalCounters(counters,telRad))
	sim.add(ch.Yield(200,1000))
	
	sig = sim.run(mesh=True,att=True)
	sig.set_shower_energy(esh)
	ch.write_ei_file(sig, outFile)
	
	print("--- Time Elapsed: {} s".format( time.time() - startT))
	
if __name__ == "__main__":
	main()

