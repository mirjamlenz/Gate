/*----------------------
  GATE version name: gate_v6

  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See GATE/LICENSE.txt for further details
  ----------------------*/

#include "GateSourceGPUVoxellizedMessenger.hh"
#include "GateSourceGPUVoxellized.hh"
#include "G4UIcmdWithAnInteger.hh"

//----------------------------------------------------------------------------------------
GateSourceGPUVoxellizedMessenger::GateSourceGPUVoxellizedMessenger(GateSourceGPUVoxellized* source)
  : GateSourceVoxellizedMessenger(source), m_gpu_source(source)
{ 
  m_attach_to_cmd = new G4UIcmdWithAString((GetDirectoryName()+"attachPhantomTo").c_str(),this);
  m_attach_to_cmd->SetGuidance("Set the name of the attached voxelized volume");

  m_gpu_buffer_size_cmd = new G4UIcmdWithAnInteger((GetDirectoryName()+"setGPUBufferSize").c_str(),this);
  m_gpu_buffer_size_cmd->SetGuidance("Set the number of particles generated by the gpu at one time");

}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
GateSourceGPUVoxellizedMessenger::~GateSourceGPUVoxellizedMessenger()
{
}
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
void GateSourceGPUVoxellizedMessenger::SetNewValue(G4UIcommand* command,G4String newValue)
{ 
  if (command == m_attach_to_cmd) m_gpu_source->AttachToVolume(newValue);
  if (command == m_gpu_buffer_size_cmd) m_gpu_source->SetGPUBufferSize(m_gpu_buffer_size_cmd->GetNewIntValue(newValue));
  GateSourceVoxellizedMessenger::SetNewValue(command,newValue);
}
//----------------------------------------------------------------------------------------

