/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/errl/plugins/errludparserfactory.H $                  */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2012,2014              */
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
#ifndef ERRL_UDPARSERFACTORY_H
#define ERRL_UDPARSERFACTORY_H

/**
 *  @file errludparserfactory.H
 *
 *  Defines the ErrlUserDetailsParserFactory base class
 */
#include "errluserdetails.H"
#include <map>

namespace ERRORLOG
{

/**
 * @class ErrlUserDetailsParserFactory
 *
 * This is a factory that produces ErrlUserDetailsParser objects. If a component
 * creates user detail data then it should derive a class from this and register
 * all of its ErrlUserDetailsParser classes with it. The parse file in the
 * plugins directory can then call the factory to create parser objects to parse
 * any user detail data.
 */
class ErrlUserDetailsParserFactory
{
public:
    /**
     *  @brief Constructor
     */
    ErrlUserDetailsParserFactory()
    {
    }

    /**
     *  @brief Destructor
     */
    ~ErrlUserDetailsParserFactory()
    {
    }

    /**
     *  @brief Register a class derived from ErrlUserDetailsParser with the
     *         factory that can parse user detail data with the specified
     *         subsection.
     *
     *  @param i_subSection The user detail data subsection that is parsed by
     *                      class T
     */
    template<typename T>
    void registerParser(const errlsubsec_t i_subSection)
    {
        iv_createObjectMap[i_subSection] = &(createParserObject<T>);
    }

    /**
     *  @brief Create an object of type ErrlUserDetailsParser that can parse
     *         user detail data with the specified subsection. The returned
     *         pointer will be polymorphic and will actually point to a type
     *         derived from ErrlUserDetailsParser.
     *
     *  @param i_subSection The user detail data subsection to parse
     *
     *  @return ErrlUserDetailsParser *
     *      Pointer to a ErrlUserDetailsParser object. Null if there is no
     *      parser registered for the specified subsection. The user must delete
     *      the object after use.
     */
    ErrlUserDetailsParser * createParser(const errlsubsec_t i_subSection)
    {
        std::map<errlsubsec_t, createParserObject_t>::const_iterator itr =
            iv_createObjectMap.find(i_subSection);

        if (itr != iv_createObjectMap.end())
        {
            // Call the object creator function (createParserObject)
            return((itr->second)());
        }
        return NULL;
    }

private:
    /**
     *  @brief Creates a ErrlUserDetailsParser of the specified derived type
     *
     *  @return ErrlUserDetailsParser * Pointer to newly created object
     */
    template<typename T>
    static ErrlUserDetailsParser * createParserObject()
    {
        return new T();
    }

    // Function pointer type of createParserObject
    typedef ErrlUserDetailsParser *(*createParserObject_t)();

    // Map of subsections and their createobject function
    std::map<errlsubsec_t, createParserObject_t> iv_createObjectMap;

    // Disabled
    ErrlUserDetailsParserFactory(const ErrlUserDetailsParserFactory &);
    ErrlUserDetailsParserFactory & operator=(
        const ErrlUserDetailsParserFactory &);
};

}

#endif

