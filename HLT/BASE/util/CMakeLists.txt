# **************************************************************************
# * Copyright(c) 1998-2014, ALICE Experiment at CERN, All rights reserved. *
# *                                                                        *
# * Author: The ALICE Off-line Project.                                    *
# * Contributors are mentioned in the code where appropriate.              *
# *                                                                        *
# * Permission to use, copy, modify and distribute this software and its   *
# * documentation strictly for non-commercial purposes is hereby granted   *
# * without fee, provided that the above copyright notice appears in all   *
# * copies and that both the copyright notice and this permission notice   *
# * appear in the supporting documentation. The authors make no claims     *
# * about the suitability of this software for any purpose. It is          *
# * provided "as is" without express or implied warranty.                  *
# **************************************************************************

# Module
set(MODULE AliHLTUtil)

# Module include folder
include_directories(${AliRoot_SOURCE_DIR}/HLT/BASE/util
                   )

# Additional include folders in alphabetical order except ROOT
include_directories(${ROOT_INCLUDE_DIRS}
                    ${AliRoot_SOURCE_DIR}/ANALYSIS/ANALYSISalice
                    ${AliRoot_SOURCE_DIR}/HLT/BASE
                    ${AliRoot_SOURCE_DIR}/RAW/RAWDatabase
                    ${AliRoot_SOURCE_DIR}/RAW/RAWDatarec
                    ${AliRoot_SOURCE_DIR}/STEER/AOD
                    ${AliRoot_SOURCE_DIR}/STEER/CDB
                    ${AliRoot_SOURCE_DIR}/STEER/ESD
                    ${AliRoot_SOURCE_DIR}/STEER/STEER
                    ${AliRoot_SOURCE_DIR}/STEER/STEERBase
                   )

# Sources in alphabetical order
set(SRCS
    AliHLTAgentUtil.cxx
    AliHLTBlockFilterComponent.cxx
    AliHLTCaloClusterReader.cxx
    AliHLTCompStatCollector.cxx
    AliHLTCorruptorComponent.cxx
    AliHLTDataGenerator.cxx
    AliHLTESDCaloClusterMaker.cxx
    AliHLTEsdCollectorComponent.cxx
    AliHLTESDMCEventPublisherComponent.cxx
    AliHLTESDTrackCuts.cxx
    AliHLTFilePublisher.cxx
    AliHLTFileWriter.cxx
    AliHLTGlobalBarrelTrack.cxx
    AliHLTJets.cxx
    AliHLTLoaderPublisherComponent.cxx
    AliHLTMCEvent.cxx
    AliHLTMonitoringRelay.cxx
    AliHLTObjectCompressionComponent.cxx
    AliHLTOUTPublisherComponent.cxx
    AliHLTRawReaderPublisherComponent.cxx
    AliHLTReadoutListDumpComponent.cxx
    AliHLTRecoParamComponent.cxx
    AliHLTRootFilePublisherComponent.cxx
    AliHLTRootFileStreamerComponent.cxx
    AliHLTRootFileWriterComponent.cxx
    AliHLTRootSchemaEvolutionComponent.cxx
   )

# Headers from sources
string(REPLACE ".cxx" ".h" HDRS "${SRCS}")

# Generate the dictionary
# It will create G_ARG1.cxx and G_ARG1.h / ARG1 = function first argument
get_directory_property(incdirs INCLUDE_DIRECTORIES)
generate_dictionary("${MODULE}" "${MODULE}LinkDef.h" "${HDRS}" "${incdirs}")

# Generate the ROOT map
# Dependecies
set(LIBDEPS STEERBase STEER AOD CDB ESD RAWDatabase RAWDatarec ANALYSISalice HLTbase Graf)
generate_rootmap("${MODULE}" "${LIBDEPS}" "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE}LinkDef.h")

# Add a library to the project using the specified source files
add_library(${MODULE} SHARED ${SRCS} G__${MODULE}.cxx)
target_link_libraries(${MODULE} STEERBase STEER AOD CDB ESD RAWDatabase RAWDatarec ANALYSISalice HLTbase Graf)

# Additional compilation flags
set_target_properties(${MODULE} PROPERTIES COMPILE_FLAGS "-O -g")

# System dependent: Modify the way the library is build
if(${CMAKE_SYSTEM} MATCHES Darwin)
    set_target_properties(${MODULE} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(${CMAKE_SYSTEM} MATCHES Darwin)

# Installation
install(TARGETS ${MODULE}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

 install(FILES ${HDRS} DESTINATION include)

 # dHLTdumpraw executable
# Additional include folders in alphabetical order except ROOT
include_directories(
                    ${AliRoot_SOURCE_DIR}/HLT/BASE/HOMER
                   )

 
set(SRCS
    hltdimserver.cxx
   )

add_executable(hltdimserver ${SRCS})
target_link_libraries(hltdimserver HLTbase AliHLTHOMER STEERBase XMLParser Graf3d RIO Hist Core)

# Installation of hltdimserver
install(TARGETS hltdimserver RUNTIME DESTINATION bin)