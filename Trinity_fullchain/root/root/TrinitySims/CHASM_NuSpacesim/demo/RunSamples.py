import numpy as np
import matplotlib.pyplot as plt
import CHASM as ch
import time
import os

def main():
	az = np.radians(0)
	arc = np.radians(10)
	phis = np.linspace(az,az+arc,11)

	telXs = np.linspace(50000,150000,3)
	telY = 0
	telZ = 3000
	telRad = 0.8

	xMaxs = np.array([645.669,671.31,684.853])
	nMaxs = np.array([1.05059e+07,2.2575e+07,3.37649e+07])
	x0s = np.array([148.87,623.354,651.931])
	lams = np.array([190.688,2040.58,3023.27])

	for i,telX in enumerate(telXs):
		zn = np.arctan(telX/telZ)
		theta = zn
		for phi in phis:
			#print("theta:{:.2f},phi:{:.2f},telX:{:.2f},telY:{:.2f},telZ:{:.2f},telRad:{:.2f},xMax:{:.2f},nMax:{:.2e},x0:{:.2f},lams:{:.2f}".format(np.rad2deg(theta),np.rad2deg(phi),telX,telY,telZ,telRad,xMaxs[i],nMaxs[i],x0s[i],lams[i]))
			simShower(theta,phi,telX,telY,telZ,telRad,xMaxs[i],nMaxs[i],x0s[i],lams[i])

def simShower(theta, phi, telX, telY, telZ, telRad, xMax, nMax, x0, lam):
	startT = time.time()
	print("Running CHASM:\n")
	print("Will Simulate the following configuration:\n")

	#esh = 1e17
	
	outFile = "sampleShower_{:.3f}_{:.3f}_{:d}_{:d}_{:d}.dat".format(np.rad2deg(theta),np.rad2deg(phi),int(telX),int(telY),int(telZ))
	
	print("Polar angle of shower: {:.2e}\nPhi angle of shower: {:.2e}\nTelescope Position: X: {:.2e} Y: {:.2e} Z: {:.2e}\n".format(theta, phi, telX, telY, telZ))
	print("Radius of detector sphere: {:.2e}\nShower Properties: XMax: {:.2e} NMax: {:.2e} X0: {:.2e} Lambda: {:.2e}".format(telRad,xMax,nMax,x0,lam))
	#print("Shower Energy: {:.2e}\n".format(esh))
	
	print("Output File: {}".format(outFile))
	Nxc = 1
	Nyc = 1
	counter = np.empty([Nxc*Nyc,3])
	r = np.sqrt(telX**2 + telY**2 + telZ**2)
	print("telescope at radius {:.2e}\n".format(r))
	counter[:,0] = telX
	counter[:,1] = telY
	counter[:,2] = telZ

	sim = ch.ShowerSimulation()
	sim.add(ch.UpwardAxis(theta,phi,curved=True))
	sim.add(ch.GHShower(xMax,nMax,x0,lam))
	sim.add(ch.SphericalCounters(counter,telRad))
	sim.add(ch.Yield(200,1000))
	
	sig = sim.run(mesh=True,att=True)
	#sig.set_shower_energy(esh)
	ch.write_ei_file(sig, outFile)
	
	print("--- Time Elapsed: {} s".format( time.time() - startT))
	
if __name__ == "__main__":
	main()

