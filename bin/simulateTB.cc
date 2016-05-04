// ----------------------------------------------------------------------------
// read geometry from a config file 04-28-2016 HBP
// ----------------------------------------------------------------------------
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "HGCal/TBStandaloneSimulator/interface/DetectorConstruction.hh"
#include "HGCal/TBStandaloneSimulator/interface/PhysicsList.hh"
#include "HGCal/TBStandaloneSimulator/interface/PrimaryGeneratorAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/RunAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/EventAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/SteppingAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/SteppingVerbose.hh"
#include "HGCal/TBStandaloneSimulator/interface/TBGeometry.h"

#include "TSystem.h"
#include "TString.h"

int main(int argc,char** argv)
{
  // A hack to avoid compiler warning
  int hack = CLHEP::HepRandomGenActive;
  hack = argc;
  argc = hack;

  if ( argc < 2 )
    {
      std::cout << "Usage: " << std::endl
		<< "  simulateTB <macro file> <geometry file>" 
		<< std::endl
		<< std::endl
		<< "OR supply a config file with a n extension other than .mac" 
		<< std::endl
		<< std::endl
		<< "  simulateTB " << "\x1b[1;31;48m<config file>\x1b[0m" 
		<< std::endl
		<< std::endl
		<< "  containing the keyword/value pairs:"
		<< std::endl
		<< "     macro       <macro file>"
		<< std::endl
		<< "     geometry    <geometry file>"
		<< std::endl
		<< "     savetracks  true|false"    
		<< std::endl;
      exit(0);
    }

  G4String macroFile = gSystem->ExpandPathName(argv[1]);
  G4String geomFile("");
  TBConfig config;

  // check if the first file is a macro file.
  // if it is, we need another argument
  if ( TString(macroFile.c_str()).EndsWith(".mac") )
    {
      if ( argc < 3 )
	{
	  std::cout << "Usage: " << std::endl
		    << "  simulateTB " << macroFile << " <geometry file>" 
		    << std::endl;
	    exit(0);
	}
      // Geometry config file
      geomFile = gSystem->ExpandPathName(argv[2]); 
      config.macro = macroFile;
      config.geometry = geomFile;
    }
  else
    {
      // Assume this is a config file
      config = TBConfig(macroFile);
    }
  G4cout << config << G4endl;

  macroFile = config.macro;
  geomFile  = config.geometry;

  TBGeometry geometry(geomFile);

  // Choose the Random engine
  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);
  
  // User Verbose output class
  G4VSteppingVerbose::SetInstance(new SteppingVerbose);
     
  // Construct the default run manager
  G4RunManager * runManager = new G4RunManager;

  // Set mandatory initialization classes

  runManager->SetUserInitialization(new DetectorConstruction(geometry, 
							     config));
  runManager->SetUserInitialization(new PhysicsList);

  // Set user action classes
  int model=geometry.model();
  double eta=0;
  runManager->SetUserAction(new PrimaryGeneratorAction(model, eta));
  runManager->SetUserAction(new RunAction);
  runManager->SetUserAction(new EventAction);
  runManager->SetUserAction(new SteppingAction);
  
  // Initialize G4 kernel
  runManager->Initialize();
  
  // Initialize visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Start User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  G4String command = "/control/execute " + macroFile; 
  UImanager->ApplyCommand(command);
 
  delete visManager;
  delete runManager;
  return 0;
}

