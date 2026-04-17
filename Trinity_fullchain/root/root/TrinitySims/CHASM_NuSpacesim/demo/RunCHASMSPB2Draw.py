import numpy as np
import matplotlib.pyplot as plt
import CHASM as ch
import sys
import time
import os
def main():
	startT = time.time()
	args = sys.argv
	print("Running CHASM:\n")
	print("Will Simulate the following configuration:\n")
	theta = float(args[1])
	phi = float(args[2])
	telX = float(args[3])
	telY = float(args[4])
	telZ = float(args[5])

	telRad = float(args[6])
	
	xMax = float(args[7])
	nMax = float(args[8])
	x0 = float(args[9])
	lam = float(args[10])
	
	esh = float(args[11])
	
	outFile = args[12]
	
	print("Polar angle of shower: {:.2e}\nPhi angle of shower: {:.2e}\nTelescope Position: X: {:.2e} Y: {:.2e} Z: {:.2e}\n".format(theta, phi, telX, telY, telZ))
	print("Radius of detector sphere: {:.2e}\nShower Properties: XMax: {:.2e} NMax: {:.2e} X0: {:.2e} Lambda: {:.2e}".format(telRad,xMax,nMax,x0,lam))
	print("Shower Energy: {:.2e}\n".format(esh))
	
	print("Output File: {}".format(outFile))


	Nxc = 20
	Nyc = 20
	angularSep = np.radians(0.0001)
	phiRange = np.linspace(phi-angularSep,phi+angularSep,Nxc)
	thetaRange = np.linspace(theta-angularSep,theta+angularSep,Nyc)
	
	
	counter = np.empty([Nxc*Nyc,3])

	thetaCtr, phiCtr = np.meshgrid(thetaRange,phiRange)

	rDetector = np.sqrt(telX**2+telY**2+telZ**2)

	counter[:,0] = rDetector*np.sin(thetaCtr.flatten())*np.cos(phiCtr.flatten());
	counter[:,1] = rDetector*np.sin(thetaCtr.flatten())*np.sin(phiCtr.flatten());
	counter[:,2] = rDetector*np.cos(thetaCtr.flatten());

	sim = ch.ShowerSimulation()
	sim.add(ch.UpwardAxis(theta,phi,curved=True))
	sim.add(ch.GHShower(xMax,nMax,x0,lam))
	sim.add(ch.SphericalCounters(counter,0.03))
	sim.add(ch.Yield(200,1000))
	
	sig = sim.run(mesh=True,att=True)
	sig.set_shower_energy(esh)
#	ch.write_ei_file(sig, outFile)
	
	print("--- Time Elapsed: {} s".format( time.time() - startT))
	fig = plt.figure()
	cx = rDetector*phiCtr.flatten()*1e-3
	cy = rDetector*thetaCtr.flatten()*1e-3
	h2d = plt.hist2d(cx-cx.mean(),cy-cy.mean(),weights=sig.photons.sum(axis=2).sum(axis=1),bins=Nxc)
	plt.colorbar(label = 'Number of Cherenkov Photons / m^2')
	ax=plt.gca()
	ax.set_aspect('equal')
	outPNG=os.path.splitext(outFile)[0]
	outPNG = outPNG+".png"
	plt.savefig(outPNG)
#	plt.show()

#	travel_vectors = sig.counters.travel_vectors(sig.source_points)
#	travel_r = np.sqrt((travel_vectors**2).sum(axis = -1))
#	cx = travel_vectors[:,:,0] / travel_r
#	cy = travel_vectors[:,:,1] / travel_r
#	cz = np.sqrt(1.0-cx**2-cy**2)

#	fig2 = plt.figure()
#	hist = plt.hist(cz.flatten(), bins=100, edgecolor='black')
#	ax2= plt.gca();
#	ax2.scatter(cx.flatten(), cz.flatten(),s=5)
#	plt.show()


#	ax = plt.gca()
#	ax.scatter(sig.y.flatten(),sig.x.flatten())
#	plt.show()
	
if __name__ == "__main__":
	main()

#Nxc = 1
#Nyc = 1


#counter = np.empty([Nxc*Nyc,3])
#counter[:,0] = r * np.sin(theta) * np.cos(phi)
#counter[:,1] = r * np.sin(theta) * np.sin(phi)
#counter[:,2] = r * np.cos(theta)

#sim = ch.ShowerSimulation()
#sim.add(ch.UpwardAxis(theta,phi,curved=True))
#sim.add(ch.GHShower(750.,3e8,0.,65.)) #Tau of 1e18eV
#sim.add(ch.SphericalCounters(counter, 1.))
#sim.add(ch.Yield(200,1000))
#sig = sim.run(mesh=True)

#print("Sim Finished")

