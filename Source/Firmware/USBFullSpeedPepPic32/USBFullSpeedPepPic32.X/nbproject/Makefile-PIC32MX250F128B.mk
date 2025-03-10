#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-PIC32MX250F128B.mk)" "nbproject/Makefile-local-PIC32MX250F128B.mk"
include nbproject/Makefile-local-PIC32MX250F128B.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=PIC32MX250F128B
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../usb_device.c ../usb_function_generic.c ../usb_descriptors.c ../main.c ../bitsettings.c ../ProcessCommand.c ../UtSleepFirmware.c ../../../UtilsPep/Shared/UtPepLogic.c ../PepDevice.c ../UtTimeDelay.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1472/usb_device.o ${OBJECTDIR}/_ext/1472/usb_function_generic.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/bitsettings.o ${OBJECTDIR}/_ext/1472/ProcessCommand.o ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o ${OBJECTDIR}/_ext/450498885/UtPepLogic.o ${OBJECTDIR}/_ext/1472/PepDevice.o ${OBJECTDIR}/_ext/1472/UtTimeDelay.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1472/usb_device.o.d ${OBJECTDIR}/_ext/1472/usb_function_generic.o.d ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d ${OBJECTDIR}/_ext/1472/main.o.d ${OBJECTDIR}/_ext/1472/bitsettings.o.d ${OBJECTDIR}/_ext/1472/ProcessCommand.o.d ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o.d ${OBJECTDIR}/_ext/450498885/UtPepLogic.o.d ${OBJECTDIR}/_ext/1472/PepDevice.o.d ${OBJECTDIR}/_ext/1472/UtTimeDelay.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1472/usb_device.o ${OBJECTDIR}/_ext/1472/usb_function_generic.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/bitsettings.o ${OBJECTDIR}/_ext/1472/ProcessCommand.o ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o ${OBJECTDIR}/_ext/450498885/UtPepLogic.o ${OBJECTDIR}/_ext/1472/PepDevice.o ${OBJECTDIR}/_ext/1472/UtTimeDelay.o

# Source Files
SOURCEFILES=../usb_device.c ../usb_function_generic.c ../usb_descriptors.c ../main.c ../bitsettings.c ../ProcessCommand.c ../UtSleepFirmware.c ../../../UtilsPep/Shared/UtPepLogic.c ../PepDevice.c ../UtTimeDelay.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

# The following macros may be used in the pre and post step lines
Device=PIC32MX250F128B
ProjectDir="C:\git\PEP\Source\Firmware\USBFullSpeedPepPic32\USBFullSpeedPepPic32.X"
ProjectName=USBFullSpeedPepPic32
ConfName=PIC32MX250F128B
ImagePath="dist\PIC32MX250F128B\${IMAGE_TYPE}\USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ImageDir="dist\PIC32MX250F128B\${IMAGE_TYPE}"
ImageName="USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-PIC32MX250F128B.mk dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: [${MP_CC_DIR}\pic32-objdump -S ${ImageDir}\${PROJECTNAME}.${IMAGE_TYPE}.elf > ${ImageDir}\${PROJECTNAME}.${IMAGE_TYPE}.lst ]"
	@${MP_CC_DIR}\pic32-objdump -S ${ImageDir}\${PROJECTNAME}.${IMAGE_TYPE}.elf > ${ImageDir}\${PROJECTNAME}.${IMAGE_TYPE}.lst 
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=32MX250F128B
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1472/usb_device.o: ../usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/usb_device.o.d" -o ${OBJECTDIR}/_ext/1472/usb_device.o ../usb_device.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/usb_function_generic.o: ../usb_function_generic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_function_generic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_function_generic.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_function_generic.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/usb_function_generic.o.d" -o ${OBJECTDIR}/_ext/1472/usb_function_generic.o ../usb_function_generic.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/bitsettings.o: ../bitsettings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/bitsettings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/bitsettings.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bitsettings.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/bitsettings.o.d" -o ${OBJECTDIR}/_ext/1472/bitsettings.o ../bitsettings.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/ProcessCommand.o: ../ProcessCommand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProcessCommand.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProcessCommand.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/ProcessCommand.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/ProcessCommand.o.d" -o ${OBJECTDIR}/_ext/1472/ProcessCommand.o ../ProcessCommand.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/UtSleepFirmware.o: ../UtSleepFirmware.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/UtSleepFirmware.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/UtSleepFirmware.o.d" -o ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o ../UtSleepFirmware.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/450498885/UtPepLogic.o: ../../../UtilsPep/Shared/UtPepLogic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/450498885" 
	@${RM} ${OBJECTDIR}/_ext/450498885/UtPepLogic.o.d 
	@${RM} ${OBJECTDIR}/_ext/450498885/UtPepLogic.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/450498885/UtPepLogic.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/450498885/UtPepLogic.o.d" -o ${OBJECTDIR}/_ext/450498885/UtPepLogic.o ../../../UtilsPep/Shared/UtPepLogic.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/PepDevice.o: ../PepDevice.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PepDevice.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PepDevice.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/PepDevice.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/PepDevice.o.d" -o ${OBJECTDIR}/_ext/1472/PepDevice.o ../PepDevice.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/UtTimeDelay.o: ../UtTimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/UtTimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/UtTimeDelay.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/UtTimeDelay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG   -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/UtTimeDelay.o.d" -o ${OBJECTDIR}/_ext/1472/UtTimeDelay.o ../UtTimeDelay.c  -legacy-libc --verbose
	
else
${OBJECTDIR}/_ext/1472/usb_device.o: ../usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/usb_device.o.d" -o ${OBJECTDIR}/_ext/1472/usb_device.o ../usb_device.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/usb_function_generic.o: ../usb_function_generic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_function_generic.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_function_generic.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_function_generic.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/usb_function_generic.o.d" -o ${OBJECTDIR}/_ext/1472/usb_function_generic.o ../usb_function_generic.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/bitsettings.o: ../bitsettings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/bitsettings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/bitsettings.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bitsettings.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/bitsettings.o.d" -o ${OBJECTDIR}/_ext/1472/bitsettings.o ../bitsettings.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/ProcessCommand.o: ../ProcessCommand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/ProcessCommand.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/ProcessCommand.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/ProcessCommand.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/ProcessCommand.o.d" -o ${OBJECTDIR}/_ext/1472/ProcessCommand.o ../ProcessCommand.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/UtSleepFirmware.o: ../UtSleepFirmware.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/UtSleepFirmware.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/UtSleepFirmware.o.d" -o ${OBJECTDIR}/_ext/1472/UtSleepFirmware.o ../UtSleepFirmware.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/450498885/UtPepLogic.o: ../../../UtilsPep/Shared/UtPepLogic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/450498885" 
	@${RM} ${OBJECTDIR}/_ext/450498885/UtPepLogic.o.d 
	@${RM} ${OBJECTDIR}/_ext/450498885/UtPepLogic.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/450498885/UtPepLogic.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/450498885/UtPepLogic.o.d" -o ${OBJECTDIR}/_ext/450498885/UtPepLogic.o ../../../UtilsPep/Shared/UtPepLogic.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/PepDevice.o: ../PepDevice.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/PepDevice.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/PepDevice.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/PepDevice.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/PepDevice.o.d" -o ${OBJECTDIR}/_ext/1472/PepDevice.o ../PepDevice.c  -legacy-libc --verbose
	
${OBJECTDIR}/_ext/1472/UtTimeDelay.o: ../UtTimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/UtTimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/UtTimeDelay.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/UtTimeDelay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION) -I".." -I"../../.." -Wall -MMD -MF "${OBJECTDIR}/_ext/1472/UtTimeDelay.o.d" -o ${OBJECTDIR}/_ext/1472/UtTimeDelay.o ../UtTimeDelay.c  -legacy-libc --verbose
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)    -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -legacy-libc -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref,--warn-section-align,--verbose 
else
dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}      -legacy-libc -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--gc-sections,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--cref,--warn-section-align,--verbose
	${MP_CC_DIR}\\pic32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/USBFullSpeedPepPic32.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/PIC32MX250F128B
	${RM} -r dist/PIC32MX250F128B

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
