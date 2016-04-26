#include "G4RunManager.hh"
#include "G4UImanager.hh"

#include "Randomize.hh"

#include "HGCal/TBStandaloneSimulator/interface/DetectorConstruction.hh"
#include "HGCal/TBStandaloneSimulator/interface/PhysicsList.hh"
#include "HGCal/TBStandaloneSimulator/interface/PrimaryGeneratorAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/RunAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/EventAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/SteppingAction.hh"
#include "HGCal/TBStandaloneSimulator/interface/SteppingVerbose.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

int main(int argc,char** argv)
{
  // Choose the Random engine
  CLHEP::HepRandom::setTheEngine(new CLHEP::RanecuEngine);
  
  // User Verbose output class
  G4VSteppingVerbose::SetInstance(new SteppingVerbose);
     
  // Construct the default run manager
  G4RunManager * runManager = new G4RunManager;

  // Set mandatory initialization classes
  int version=110;
  //int version=DetectorConstruction::v_HGCALEE_TB;
  //int model=DetectorConstruction::m_FULLSECTION;
  //int model=DetectorConstruction::m_BOXWITHCRACK_100;
  int model=DetectorConstruction::m_2016TB;
  //int model=DetectorConstruction::m_SIMPLE_100;

  double eta=0;

  if(argc>2) version=atoi(argv[2]);
  if(argc>3) model=atoi(argv[3]);
  if(argc>4) eta=atof(argv[4]);

  std::cout << "-- Running version " << version << " model " << model << std::endl;

  std::string absThickW="1.75,1.75,1.75,1.75,1.75,2.8,2.8,2.8,2.8,2.8,4.2,4.2,4.2,4.2,4.2";
  std::string absThickPb="1,1,1,1,1,2.1,2.1,2.1,2.1,2.1,4.4,4.4,4.4,4.4";
  std::string dropLayers="";
  if(argc>5) absThickW = argv[5];
  if(argc>6) absThickPb = argv[6];
  if(argc>7) dropLayers = argv[7];

  runManager->SetUserInitialization(new DetectorConstruction(version,model,absThickW,absThickPb,dropLayers));
  runManager->SetUserInitialization(new PhysicsList);

  // Set user action classes
  runManager->SetUserAction(new PrimaryGeneratorAction(model,eta));
  runManager->SetUserAction(new RunAction);
  runManager->SetUserAction(new EventAction);
  runManager->SetUserAction(new SteppingAction);
  
  // Initialize G4 kernel
  runManager->Initialize();
  
  // Initialize visualization
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  G4String fileName;
  if (argc>1) fileName = argv[1];
  if (argc!=1)   // batch mode
    {    
      std::cout << " ====================================== " << std::endl
		<< " ========  Running batch mode ========= " << std::endl
		<< " ====================================== " << std::endl;
      G4String command = "/control/execute ";
      UImanager->ApplyCommand(command+fileName);
    }
  else
    {
      std::cout << " ====================================== " << std::endl
		<< " ====  Running interactive display ==== " << std::endl
		<< " ====================================== " << std::endl;
      G4UIExecutive* ui = new G4UIExecutive(argc, argv);
      UImanager->ApplyCommand("/control/execute vis.mac"); 
      if (ui->IsGUI())
        UImanager->ApplyCommand("/control/execute gui.mac");
      ui->SessionStart();
      delete ui;
    }
  delete visManager;
  delete runManager;
  return 0;
}

