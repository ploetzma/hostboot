/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/htmgt/tmgtutility.C $                                 */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014                             */
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

#include <htmgt/htmgt.H>
#include <htmgt/htmgt_reasoncodes.H>
#include "htmgt_activate.H"
#include "htmgt_cfgdata.H"
#include "htmgt_utility.H"
#ifndef __HOSTBOOT_RUNTIME
#include "genPstate.H"
#endif
#include "htmgt_memthrottles.H"
#include "htmgt_poll.H"
#include <devicefw/userif.H>

//  Targeting support
#include <targeting/common/commontargeting.H>
#include <targeting/common/utilFilter.H>
#include <targeting/common/attributes.H>
#include <targeting/common/targetservice.H>


#include <sys/time.h>

namespace HTMGT
{

    const uint32_t HTMGT_DELAY_BEFORE_COMM = 5;


    // Move the OCCs to active state or log unrecoverable error and
    // stay in safe mode
    void processOccStartStatus(const bool i_startCompleted,
                               TARGETING::Target * i_failedOccTarget)
    {
        errlHndl_t l_err = NULL;
        uint32_t l_huid = 0;
        if (i_failedOccTarget)
        {
            l_huid = TARGETING::get_huid(i_failedOccTarget);
        }
        TMGT_INF("processOccStartStatus(Start Success=%c, failedOcc=0x%08X)",
                 i_startCompleted?'y':'n', l_huid);
        if (i_startCompleted)
        {
            // Query functional OCCs
            const uint8_t numOccs = occMgr::instance().buildOccs();
            if (numOccs > 0)
            {
                if (NULL != occMgr::instance().getMasterOcc())
                {
                    do
                    {
                        // Delay before communication with OCCs to make sure
                        // they are ready (since there is no initial attention)
                        nanosleep(HTMGT_DELAY_BEFORE_COMM, 0);

#ifndef __HOSTBOOT_RUNTIME
                        if (false == occMgr::instance().iv_configDataBuilt)
                        {
                            // Build pstate tables (once per IPL)
                            l_err = genPstateTables();
                            if(l_err)
                            {
                                break;
                            }

                            // Calc memory throttles (once per IPL)
                            calcMemThrottles();

                            occMgr::instance().iv_configDataBuilt = true;
                        }
#endif

                        // Send poll to establish comm
                        TMGT_INF("Send initial poll to all OCCs to"
                                 " establish comm");
                        errlHndl_t l_err = sendOccPoll();
                        if (l_err)
                        {
                            // Continue even if failed (poll will be retried)
                            ERRORLOG::errlCommit(l_err, HTMGT_COMP_ID);
                            l_err = NULL;
                        }

                        // Send ALL config data
                        sendOccConfigData();

                        // Wait for all OCCs to go active
                        l_err = waitForOccsActive();
                        if ( l_err )
                        {
                            break;
                        }

                        // Set active sensors for all OCCs,
                        // so BMC can start communication with OCCs
                        l_err = setOccActiveSensors();
                        if (l_err)
                        {
                            // Continue even if failed to update sensor
                            ERRORLOG::errlCommit(l_err, HTMGT_COMP_ID);
                            l_err = NULL;
                        }
                    } while(0);
                }
                else
                {
                    TMGT_ERR("Unable to find any Master capable OCCs");
                    /*@
                     * @errortype
                     * @reasoncode      HTMGT_RC_OCC_MASTER_NOT_FOUND
                     * @moduleid        HTMGT_MOD_LOAD_START_STATUS
                     * @userdata1[0:7]  number of OCCs
                     * @devdesc         No OCC master was found
                     */
                    bldErrLog(l_err, HTMGT_MOD_LOAD_START_STATUS,
                              HTMGT_RC_OCC_MASTER_NOT_FOUND,
                              numOccs, 0, 0, 0,
                              ERRORLOG::ERRL_SEV_INFORMATIONAL);
                }
            }
            else
            {
                TMGT_ERR("Unable to find any functional OCCs");
                /*@
                 * @errortype
                 * @reasoncode      HTMGT_RC_OCC_UNAVAILABLE
                 * @moduleid        HTMGT_MOD_LOAD_START_STATUS
                 * @userdata1[0:7]  load/start completed
                 * @devdesc         No functional OCCs were found
                 */
                bldErrLog(l_err, HTMGT_MOD_LOAD_START_STATUS,
                          HTMGT_RC_OCC_UNAVAILABLE,
                          i_startCompleted, 0, 0, 0,
                          ERRORLOG::ERRL_SEV_INFORMATIONAL);
            }
        }
        else
        {
            TMGT_ERR("All OCCs were not loaded/started successfully");
            /*@
             * @errortype
             * @reasoncode      HTMGT_RC_OCC_START_FAIL
             * @moduleid        HTMGT_MOD_LOAD_START_STATUS
             * @userdata1       Failing OCC HUID
             * @devdesc         OCCs were not loaded/started successfully
             */
            bldErrLog(l_err, HTMGT_MOD_LOAD_START_STATUS,
                      HTMGT_RC_OCC_START_FAIL,
                      l_huid, 0, 0, 0,
                      ERRORLOG::ERRL_SEV_INFORMATIONAL);
        }

        if (NULL != l_err)
        {
            TMGT_ERR("OCCs not all active.  System will stay in safe mode");
            // TODO: RTC 109066
            //stopAllOccs();

            // Update error log to unrecoverable and set SRC
            // to indicate the system will remain in safe mode
            /*@
             * @errortype
             * @reasoncode      HTMGT_RC_OCC_CRIT_FAILURE
             * @moduleid        HTMGT_MOD_LOAD_START_STATUS
             * @userdata1[0:7]  load/start completed
             * @devdesc         OCCs did not all reach active state,
             *                  system will be in Safe Mode
             */
            bldErrLog(l_err, HTMGT_MOD_LOAD_START_STATUS,
                      HTMGT_RC_OCC_CRIT_FAILURE,
                      i_startCompleted, 0, 0, 1,
                      ERRORLOG::ERRL_SEV_UNRECOVERABLE);

            // Add level 2 support callout
            l_err->addProcedureCallout(HWAS::EPUB_PRC_LVL_SUPP,
                                       HWAS::SRCI_PRIORITY_MED);
            // Add HB firmware callout
            l_err->addProcedureCallout(HWAS::EPUB_PRC_HB_CODE,
                                       HWAS::SRCI_PRIORITY_MED);

            ERRORLOG::errlCommit(l_err, HTMGT_COMP_ID);
        }

    } // end processOccStartStatus()



    // Notify HTMGT that an OCC has an error to report
    void processOccError(TARGETING::Target * i_occTarget)
    {
        const uint32_t l_huid = i_occTarget->getAttr<TARGETING::ATTR_HUID>();
        TMGT_INF("processOccError(HUID=0x%08X) called", l_huid);
        // TODO RTC 109224

    } // end processOccError()



    // Notify HTMGT that an OCC has failed and needs to be reset
    void processOccReset(TARGETING::Target * i_failedOccTarget)
    {
        const uint32_t l_huid =
            i_failedOccTarget->getAttr<TARGETING::ATTR_HUID>();
        TMGT_INF("processOccReset(HUID=0x%08X) called", l_huid);
        // TODO RTC 115296

    } // end processOccReset()


}

