/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/usr/hwpf/fapi/fapiUtil.H $                        */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
// $Id: fapiUtil.H,v 1.21 2014/09/16 14:47:23 pragupta Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/hwpf/working/fapi/fapiUtil.H,v $

/**
 *  @file fapiUtil.H
 *
 *  @brief Defines utility functions that the platform code must implement.
 */

/*
 * Change Log ******************************************************************
 * Flag     Defect/Feature  User        Date        Description
 * ------   --------------  ----------  ----------- ----------------------------
 *                          mjjones     04/13/2011  Created.
 *                          camvanng    05/31/2011  Removed fapiOutputx macros
 *                          mjjones     06/30/2011  Removed #include
 *                          mjjones     07/05/2011  Removed rogue tab
 *                          camvanng    09/06/2011  Added fapiLogError
 *                          mjjones     09/14/2011  Prepended fapi to delay
 *                          mjjones     10/05/2011  Added fapiCheckType
 *                          mjjones     10/13/2011  Added extern "C" to functions
 *                          camvanng    10/14/2011  Added fapiLoadInitFile &
 *                                                  fapiUnloadInitFile
 *                          camvanng    11/15/2011  Pass target to fapiLoadInitFile
 *                                                  and doxygen changes;
 *                                                  needed by cronus
 *                          mjjones     08/13/2012  Add fapiSpecialWakeup
 *                          mjjones     09/24/2013  Update fapiLogError to
 *                                                  accept another parameter
 *                          dcrowell    10/14/2013  Add explicit inline
 *                          pragupta    09/16/2014  Add fapi gen hash function
 */

#ifndef FAPIUTIL_H_
#define FAPIUTIL_H_

#include <stdint.h>
#include <stddef.h>
#include <fapiReturnCode.H>

namespace fapi
{
    namespace hash
    {
        enum validHashes { CRC32 };
    }
};

#include <fapiPlatUtil.H>

// Defines for endian convertion
#define FAPI_BE16TOH(x) FAPI_PLAT_BE16TOH(x)
#define FAPI_LE16TOH(x) FAPI_PLAT_LE16TOH(x)
#define FAPI_HTOBE16(x) FAPI_PLAT_HTOBE16(x)
#define FAPI_HTOLE16(x) FAPI_PLAT_HTOLE16(x)

#define FAPI_BE32TOH(x) FAPI_PLAT_BE32TOH(x)
#define FAPI_LE32TOH(x) FAPI_PLAT_LE32TOH(x)
#define FAPI_HTOBE32(x) FAPI_PLAT_HTOBE32(x)
#define FAPI_HTOLE32(x) FAPI_PLAT_HTOLE32(x)

#define FAPI_BE64TOH(x) FAPI_PLAT_BE64TOH(x)
#define FAPI_LE64TOH(x) FAPI_PLAT_LE64TOH(x)
#define FAPI_HTOBE64(x) FAPI_PLAT_HTOBE64(x)
#define FAPI_HTOLE64(x) FAPI_PLAT_HTOLE64(x)

// Defines for platform-specific memory allocation
#define fapiMalloc(s) fapiPlatMalloc(s)
#define fapiFree(p) fapiPlatFree(p)

#define FAPI_GEN_HASH(type, data, size) FAPI_PLAT_GEN_HASH(type,data,size)
// It is an eCMD requirement that these functions have a "C" symbol
// because they may be used from a dynamically linked shared library
extern "C"
{

/**
 * @brief Assert that an expression is true. Aborting the process if false.
 *
 * @note Implemented by platform code
 *
 * @param[in] i_expression If not true then process should be aborted
 */
void fapiAssert(bool i_expression);

/**
 * @brief Delay this thread. Hostboot will use the nanoseconds parameter
 * and make a syscall to nanosleep. While in the syscall, the hostboot
 * kernel will continue to consume CPU cycles as it looks for a runnable
 * task.  When the delay time expires, the task becomes runnable and will soon
 * return from the syscall.  Callers of delay() in the hostboot environment
 * will likely have to know the mHz clock speed they are running on and
 * compute a non-zero value for i_nanoSeconds.
 *
 * On the FSP, it was sometimes acceptable to just provide zero for the
 * sleep delay time, causing the task to yield its time slice. By the
 * time the calling task could run again, it was pretty certain enough
 * host cycles had past.  This is probably not acceptable in
 * the hostboot environment. Callers should calculate and provide a
 * sleep value in nanoseconds relative to host clock speed.
 *
 * On FSP when VBU is the target, then the i_simCycles parameter will be
 * used instead.  The FSP needs to use the simdispatcher client/server
 * API and issue a command to the awan to advance the simulation the
 * specified number of cycles.
 *
 * @param[in] i_nanoSeconds    nanoseconds to sleep
 * @param[in] i_simCycles      count of Awan cycles to advance
 *
 * @return ReturnCode. Zero on success, else platform specified error.
 */
fapi::ReturnCode fapiDelay(uint64_t i_nanoSeconds, uint64_t i_simCycles);

namespace fapi
{
/**
 * @brief Enumeration of error log severity.
 */
enum fapiErrlSeverity_t
{
    FAPI_ERRL_SEV_RECOVERED     = 0x10, //Not seen by customer
    FAPI_ERRL_SEV_PREDICTIVE    = 0x20, //Error recovered but customer will see
    FAPI_ERRL_SEV_UNRECOVERABLE = 0x40  //Unrecoverable, general
};

}

/**
 * @brief Log an error.
 *
 * This function can be called by HWP to log an error.
 *
 * @note Implemented by platform code
 *
 * @param[in,out] io_rc Reference to ReturnCode (Any references to data and error
 *            target are removed and rc value is set to success after
 *            function ends.)
 * @param[in] i_sev Fapi error log severity defaulted to unrecoverable
 * @param[in] i_unitTestError - flag to log error which does not cause a unit
 *                              test to fail.
 *
 * Example usage: <br>
 *     fapi::ReturnCode l_rc; <br>
 *     fapi::fapiErrlSeverity_t l_sev = fapi::FAPI_ERRL_SEV_UNRECOVERABLE;<br>
 *     FAPI_EXEC_HWP(l_rc, function1, i_target); <br>
 *     if (!l_rc) <br>
 *     { <br>
 *         fapiLogError(l_rc, l_sev); <br>
 *     } <br>
 *
 *     FAPI_EXEC_HWP(l_rc, function2, i_target) <br>
 *     return rc; <br>
 */
void fapiLogError(fapi::ReturnCode & io_rc,
                  fapi::fapiErrlSeverity_t i_sev =
                  fapi::FAPI_ERRL_SEV_UNRECOVERABLE,
                  bool i_unitTestError=false);

/**  @brief This function answers the question, is scand tracing turned on?
 *   The implementation of this function is specific to the platform.
 *
 *   @returns Boolean indication
 */
bool platIsScanTraceEnabled();

/**  @brief Alter the state of scand tracing.
 *   The implementation of this function is specific to the platform.
 *
 *   @param[in] i_enable True to enable or false to disable scan trace.
 *   @return  void
 */
void platSetScanTrace(bool i_enable);

/**  @brief Load the initfile
 *
 * This function can be called by a HWP to load an initfile.
 *
 * @note Implemented by platform code.  Platform code is
 * responsible for allocating any memory needed to load
 * the initfile.
 *
 * @param[in]  i_Target reference to the target
 * @param[in]  i_file the .if filename:  \<initfile\>.if
 * @param[out] o_addr address in memory where initfile is loaded
 * @param[out] o_size size of memory allocated for initfile
 *
 * @return ReturnCode. Zero on success, else platform specified error.
 */
fapi::ReturnCode fapiLoadInitFile(const fapi::Target & i_Target,
    const char * i_file, const char *& o_addr, size_t & o_size);

/**  @brief Unload the initfile
 *
 * This function can be called by a HWP to unload an initfile.
 *
 * @note Implemented by platform code.  Platform code is
 * responsible for deleting any allocated memory.
 *
 * @param[in] i_file the .if filename:  \<initfile\>.if
 * @param[in,out] io_addr address in memory where initfile is loaded
 *                set to NULL on exit
 * @param[in,out] io_size size of memory allocated for initfile
 *                set to 0 on exit
 *
 * @return ReturnCode. Zero on success, else platform specified error.
 */
fapi::ReturnCode fapiUnloadInitFile(const char * i_file, const char *& io_addr,
    size_t & io_size);

/**  @brief Set a break point in the current procedure
 *
 * This function will suspend the calling istep/procedure.
 * The procedure can be signaled to resume via a cmd through the
 * istep dispatcher.
 * @param[in] i_info  Tag to send when at breakpoint
 */

void fapiBreakPoint(uint32_t i_info);

/**
 * @brief Enable/Disable special wakeup on processor chip core(s)
 *
 * Special Wakeup Enable must be done when a HWP is doing an operation that
 * requires core(s) to be awake (e.g. modifying the Sleep-Winkle image). For
 * each Special Wakeup Enable call, there must be a subsequent Special Wakeup
 * Disable call.
 *
 * This does not apply to SCOM operations, platforms must handle Special Wakeup
 * for SCOM operations internally.
 *
 * If Special Wakeup is enabled, a core will not go to sleep (if already
 * sleeping, it is woken up). If Special Wakeup is disabled, if there are no
 * other active Enables, the core is allowed to sleep.
 *
 * @note Implemented by platform code calling the proc_cpu_special_wakeup HWP.
 *       This is a FAPI function because each platform may do different things
 *         Hostboot: Does nothing (cores cannot sleep while Hostboot running)
 *         FSP: Uses an algorithm to decide when to disable special wakeup
 *         Cronus: Does Special Wakeup enable/disable as requested
 *
 * @param[in] i_target
 *              TARGET_TYPE_PROC_CHIP: Enables/Disables Special Wakeup on all
 *                cores (EX chiplets) of the specified chip
 *              TARGET_TYPE_EX_CHIPLET: Enables/Disables Special Wakeup on the
 *                specified core (EX chiplet)
 * @param[in] i_enable true = enable. false = disable.
 *
 * @return ReturnCode. Zero on success, else platform specified error.
 */
fapi::ReturnCode fapiSpecialWakeup(const fapi::Target & i_target,
                                   const bool i_enable);

}

namespace fapi
{

/**
 * @brief Check the type of a variable
 *
 * This function can be called to check that a variable type is as expected
 */
template<typename T>
inline
void fapiCheckType(const T &) {}

}

#endif // FAPIUTIL_H_
