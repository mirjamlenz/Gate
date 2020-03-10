/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See LICENSE.md for further details
  ----------------------*/


/*
  \brief Class GateEmCalculatorActor :
  \brief
*/

#include "GateEmCalculatorActor.hh"
#include "GateMiscFunctions.hh"
#include "G4Event.hh"
#include "G4MaterialTable.hh"
#include "G4IonTable.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"

#include "G4UnitsTable.hh"
#include "G4ProductionCutsTable.hh"
#include "G4PhysicalConstants.hh"


//-----------------------------------------------------------------------------
/// Constructors (Prototype)
GateEmCalculatorActor::GateEmCalculatorActor(G4String name, G4int depth):
  GateVActor(name,depth)
{
  GateDebugMessageInc("Actor",4,"GateEmCalculatorActor() -- begin\n");
  //SetTypeName("EmCalculatorActor");
  //  pActor = new GateActorMessenger(this);
  ResetData();
  GateDebugMessageDec("Actor",4,"GateEmCalculatorActor() -- end\n");

  mEnergy = 100 ;
  mPartName = "proton";
  mIsGenericIon = false;

  pActorMessenger = new GateEmCalculatorActorMessenger(this);
  emcalc = new G4EmCalculator;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/// Destructor
GateEmCalculatorActor::~GateEmCalculatorActor()
{
  //  delete pActor;
  delete pActorMessenger;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Construct
void GateEmCalculatorActor::Construct()
{
  GateVActor::Construct();
  //  Callbacks
  //   EnableBeginOfRunAction(true);
  //   EnableBeginOfEventAction(true);
  //   EnablePreUserTrackingAction(true);
  //   EnableUserSteppingAction(true);
  //   ResetData();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Callback Begin of Run
/*void GateEmCalculatorActor::BeginOfRunAction(const G4Run*r)
  {
  }*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Callback Begin Event
/*void GateEmCalculatorActor::BeginOfEventAction(const G4Event*e)
  {
  }*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Callback Begin Track
/*void GateEmCalculatorActor::PreUserTrackingAction(const GateVVolume * v, const G4Track*t)
  {
  }*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Callbacks
/*void GateEmCalculatorActor::UserSteppingAction(const GateVVolume * v, const G4Step * step)
  {
  }*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Save data
void GateEmCalculatorActor::SaveData()
{
  GateVActor::SaveData();
  std::ofstream os;
  OpenFileOutput(mSaveFilename, os);

  double cut = DBL_MAX;
  double EmDEDX=0, NuclearDEDX=0, TotalDEDX=0;
  double density=0;
  double I=0;
  double eDensity=0;
  double radLength=0;
  G4double CrossSectionProcess = 0;
  G4double MuMassCoeficient = 0;
  G4String material;
  const G4MaterialTable* matTbl = G4Material::GetMaterialTable();

  const G4ParticleDefinition* gamma_definition = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  const G4ParticleDefinition* particle_definition = mIsGenericIon? GetIonDefinition() : G4ParticleTable::GetParticleTable()->FindParticle(mPartName);
  G4ProcessVector* plist = gamma_definition->GetProcessManager()->GetProcessList();
  std::vector<G4String> processNameVector;
  for (unsigned int j = 0; j < plist->size(); j++)
    {
      if ( ( (*plist)[j]->GetProcessType() == fElectromagnetic) && ((*plist)[j]->GetProcessName() != "msc"))
        {
          processNameVector.push_back((*plist)[j]->GetProcessName());
        }
    }

  os << "# Output calculted for the following parameters:\n";
  os << "# Energy\t" << mEnergy << " MeV\n";
  os << "# Particle\t" << mPartName << " " << mParticleParameters << "\n\n"; // parameters are empty unless mPartName=GenericIon
  os << "# And for the following materials\n";
  // labels
  os << "Material\t";
  os << "Density\t\t";
  os << "e-density\t";
  os << "RadLength\t";
  os << "I\t";
  os << "EM-DEDX\t\t";
  os << "Nucl-DEDX\t";
  os << "Tot-DEDX\t";
  os << "Mu_mass\n";
  // units
  os << "\t\t";
  os << "(g/cm³)\t\t";
  os << "(e-/mm³)\t";
  os << "(mm)\t\t";
  os << "(eV)\t";
  os << "(MeV.cm²/g)\t";
  os << "(MeV.cm²/g)\t";
  os << "(MeV.cm²/g)\t";
  os << "(cm²/g)\n";

  for(size_t k=0;k<G4Material::GetNumberOfMaterials();k++)
    {
      material = (*matTbl)[k]->GetName();
      density = (*matTbl)[k]->GetDensity();
      eDensity = (*matTbl)[k]->GetElectronDensity();
      radLength = (*matTbl)[k]->GetRadlen();
      I = (*matTbl)[k]->GetIonisation()->GetMeanExcitationEnergy();
      EmDEDX = emcalc->ComputeElectronicDEDX(mEnergy, particle_definition, (*matTbl)[k], cut) / density;
      NuclearDEDX = emcalc->ComputeNuclearDEDX(mEnergy, particle_definition, (*matTbl)[k]) / density;
      TotalDEDX = emcalc->ComputeTotalDEDX(mEnergy, particle_definition, (*matTbl)[k], cut) / density;
      MuMassCoeficient = 0.;
      for( size_t j = 0; j < processNameVector.size(); j++)
        {
          CrossSectionProcess = emcalc->ComputeCrossSectionPerVolume( mEnergy, particle_definition, processNameVector[j], (*matTbl)[k], cut);
          MuMassCoeficient += CrossSectionProcess / density;
        }


      // Get methods issue
      // for instance I tried:  double CSDARange = emcalc->GetDEDX(mEnergy, mPartName, material);
      // I think geometries should be initialized first and then Get methods called and then physics and source could be initialized
      // Currently all 3 initialization methods are called together, making difficult the use of GetMethods of G4EmCalculator.

      // values
      os << material << "\t\t";
      os << density*e_SI << "\t\t";
      os << eDensity << "\t";
      os << radLength << "\t\t";
      os << I/eV << "\t";
      os << EmDEDX / (MeV*cm2/g) << "\t\t";
      os << NuclearDEDX / (MeV*cm2/g) << "\t";
      os << TotalDEDX / (MeV*cm2/g) << "\t\t";
      os << MuMassCoeficient / (cm2/g) << Gateendl;
    }

  if (!os) {
    GateMessage("Output",1,"Error Writing file: " <<mSaveFilename << Gateendl);
  }
  os.flush();
  os.close();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GateEmCalculatorActor::ResetData()
{
}

//-----------------------------------------------------------------------------
const G4ParticleDefinition*
GateEmCalculatorActor::GetIonDefinition()
  {
    if (mPartName!="GenericIon"){
      // A bit late to do this here and now, but in the messenger the two options may come in in any order,
      // so it is not easy to check there. PencilBeam and TPSPencilBeam have a simlar problem (not by coincidence,
      // because I tried to keep the options in the same style.)
      GateError("Got ion parameters '" << mParticleParameters << "' but particle name " << mPartName << "!=GenericIon");
    }
    //// 4 possible arguments are Z, A, Charge, Excite Energy
    //int atomic_number = 0;
    //int atomic_mass = 0;
    //std::istringstream iss((const char*)mParticleParameters);
    //iss >> atomic_number >> atomic_mass;
    //
    // 4 possible arguments are Z, A, Charge, Excite Energy (implementation copied/adapted from GateSourcePencilBeamActor)
    G4Tokenizer next(ParticleParameters);
    atomic_number = StoI(next());
    atomic_mass = StoI(next());
    G4String sQ = next();
    if (sQ.isNull())
      {
	ion_charge = atomic_number;
      }
    else
      {
	ion_charge = StoI(sQ);
	sQ = next();
	if (sQ.isNull())
	  {
	    ion_excite_energy = 0.0;
	  }
	else
	  {
	    ion_excite_energy = StoD(sQ) * keV;
	  }
      }

    GateMessage("Output",2,"Got atomic number = " << atomic_number << " and atomic mass = " << atomic_mass << Gateendl );
    //const G4ParticleDefinition* p = G4IonTable::GetIonTable()->GetIon(atomic_number,atomic_mass,ion_charge,ion_excite_energy);
    const G4ParticleDefinition* p = G4IonTable::GetIonTable()->GetIon(atomic_number,atomic_mass,ion_excite_energy);
    GateMessage("Output",2,"particle name = " << p->GetParticleName()<< Gateendl );
    GateMessage("Output",2,"particle type = " << p->GetParticleType()<< Gateendl );
    GateMessage("Output",2,"particle pdg mass = " << p->GetPDGMass()<< Gateendl );
    return p;
  }
