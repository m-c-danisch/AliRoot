// $Id$
// Category: geometry
//
// See the class description in the header file.

#include "AliSingleModuleConstruction.h"
#include "AliSingleModuleConstructionMessenger.h"
#include "AliSDManager.h"
#include "AliGlobals.h"
#include "AliFiles.h"
#include "AliRun.h"
#include "AliModule.h"

#include "TG4GeometryManager.h"

#include <G3SensVolVector.hh>
#include <G4UImanager.hh>
//#include <G4Element.hh>
#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>

#include <TROOT.h> 
#include <TCint.h> 

G4VPhysicalVolume* AliSingleModuleConstruction::fgWorld = 0;

AliSingleModuleConstruction::AliSingleModuleConstruction(
                                G4String moduleName, G4int version,
				AliModuleType moduleType)
  : AliModuleConstruction(moduleName),
    fVersion(version),
    fType(moduleType),
    fProcessConfig(true),
    fAllLVSensitive(false)
{
//
  fSDManager = AliSDManager::Instance();

  moduleName.toLower();
  fMessenger = new AliSingleModuleConstructionMessenger(this, moduleName);
}

AliSingleModuleConstruction::AliSingleModuleConstruction(
                                const AliSingleModuleConstruction& right)
  : AliModuleConstruction(right)				
{
//
  fVersion = right.fVersion;
  fType = right.fType;
  fProcessConfig = right.fProcessConfig;
  fAllLVSensitive = right.fAllLVSensitive;
  fSDManager = right.fSDManager;

  G4String moduleName = right.fModuleName;
  moduleName.toLower();
  fMessenger = new AliSingleModuleConstructionMessenger(this, moduleName);
}

AliSingleModuleConstruction::AliSingleModuleConstruction() {
//
}

AliSingleModuleConstruction::~AliSingleModuleConstruction() {
//
  delete fMessenger;
}

// operators

AliSingleModuleConstruction& 
AliSingleModuleConstruction::operator=(const AliSingleModuleConstruction& right)
{    
  // check assignement to self
  if (this == &right) return *this;
  
  // base class assignement
  AliModuleConstruction::operator=(right);
  
  fVersion = right.fVersion;
  fType = right.fType;
  fProcessConfig = right.fProcessConfig;
  fAllLVSensitive = right.fAllLVSensitive;
  fSDManager = right.fSDManager;
  
  return *this;
}

// private methods

void AliSingleModuleConstruction::CreateSensitiveDetectors()
{
// Creates sensitive detectors.
// ---

  if (fAllLVSensitive)
    CreateSensitiveDetectors1();
  else
    CreateSensitiveDetectors2();

  // set static number of logical volumes already processed
  G4LogicalVolumeStore* pLVStore = G4LogicalVolumeStore::GetInstance();
  fSDManager->SetNofLVWithSD(pLVStore->entries());  
}   

void AliSingleModuleConstruction::CreateSensitiveDetectors1()
{ 
// Creates sensitive detectors.
// Sensitive detectors are set to all logical volumes
// ---

  G4LogicalVolumeStore* pLVStore = G4LogicalVolumeStore::GetInstance();
  G4int nofLV = pLVStore->entries();
  
  G4int nofLVWithSD = fSDManager->GetNofLVWithSD();
  
  for (G4int i=nofLVWithSD; i<nofLV; i++) {
    G4LogicalVolume* lv = (*pLVStore)[i];
    fSDManager->CreateSD(lv, fAliModule);
  }
}

void AliSingleModuleConstruction::CreateSensitiveDetectors2()
{ 
// Creates sensitive detectors.
// Sensitive detectors are set only to logical volumes
// in G3SensVolVector.
// ---

  TG4GeometryManager* pGeometryManager = TG4GeometryManager::Instance();

  G3SensVolVector pSVVector
    = pGeometryManager->GetG3SensVolVector();

  G4int nofSV = pSVVector.entries();
  if (nofSV>0)
    for (G4int isv=0; isv<nofSV; isv++) {
      G4LogicalVolume* lv = pSVVector[isv];
      fSDManager->CreateSD(lv, fAliModule);
    } 
}

// public methods 

void AliSingleModuleConstruction::Configure(const AliFiles& files)
{ 
// Executes the detector setup Root macro
// (extracted from AliRoot Config.C) and
// G4 macro.
// ---
   	  
  // filepaths and macro names 
  G4bool isStructure = (fType == kStructure);
  G4String rootFilePath 
    = files.GetRootMacroPath(fModuleName, isStructure);
  G4String g4FilePath
    = files.GetG4MacroPath(fModuleName, isStructure);
  fDataFilePath 
    = files.GetG3CallsDatPath(fModuleName, fVersion, isStructure); 
  
  // load and execute aliroot config macro
  if (fProcessConfig) {
    gROOT->LoadMacro(rootFilePath);
    G4String macroName = files.GetDefaultMacroName();
    macroName = macroName + "(";
    AliGlobals::AppendNumberToString(macroName, fVersion);
    macroName = macroName + ")";
    gInterpreter->ProcessLine(macroName);
  } 
  
  // process g4 config macro
  G4String command = "/control/execute ";
  G4UImanager* pUI = G4UImanager::GetUIpointer();  
  pUI->ApplyCommand(command + g4FilePath);
  
  // get AliModule created in Config.C macro
  fAliModule = gAlice->GetModule(fModuleName);
  if (!fAliModule) {
    G4String text = "AliSingleModuleConstruction::Configure:\n";
    text = text + "    AliModule " + fModuleName;
    text = text + " has not been found in gAlice.";
    AliGlobals::Exception(text);
  }  
}

void AliSingleModuleConstruction::Construct()
{ 
// Constructs geometry.
// ---

  // print default element table
  // const G4ElementTable* table = G4Element::GetElementTable();
  // G4cout << "Default elemnt table: " << G4endl;
  // for (G4int i=0; i<table->entries(); i++) {
  //   G4cout << *(*table)[i] << G4endl;
  // }  

  // Configure();

  // get geometry manager
  TG4GeometryManager* pGeometryManager = TG4GeometryManager::Instance();

  // register module name in the name map
  pGeometryManager->SetMapSecond(fAliModule->GetName());	

  if (fReadGeometry) {
    // create G3 geometry from g3calls.dat
    pGeometryManager->SetWriteGeometry(false);
    pGeometryManager->ReadG3Geometry(fDataFilePath);
  }
  else {
    // set geometry output stream for this module
    pGeometryManager->SetWriteGeometry(fWriteGeometry);
    if (fWriteGeometry) 
      pGeometryManager->OpenOutFile(fDataFilePath);

    // create geometry from AliRoot

    // construct materials
    fAliModule->CreateMaterials();

    // construct G3 geometry
    fAliModule->CreateGeometry();
        
    if (fWriteGeometry) 
      pGeometryManager->CloseOutFile();
  }  
  
  // construct G4 geometry
  G4VPhysicalVolume* world = pGeometryManager->CreateG4Geometry();
  if (!fgWorld) fgWorld = world; 
  
  // set the detector frame (envelope)
  // (without warning output if enevelope is not defined)
  SetDetFrame(false);

  // create sensitive detectors
  CreateSensitiveDetectors();

  // build sensitive detectors table
  fAliModule->Init();

  // construct geometry for display
  fAliModule->BuildGeometry();

  // reset TG4GeometryManager 
  pGeometryManager->ClearG3Tables();
  
  // print current total number of logical volumes
  G4cout << "Current total number of sensitive volumes: "
         << pGeometryManager->NofVolumes() << G4endl;

#ifdef ALICE_VISUALIZE
  if (GetDetFrame()) {
    // set visualization attributes
    // if detector envelope is defined
    SetDetVisibility(true);
    SetDetColour("Yellow");
  }  
#endif
}
