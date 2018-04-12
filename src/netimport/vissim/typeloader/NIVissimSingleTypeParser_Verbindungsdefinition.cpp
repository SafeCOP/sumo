/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2018 German Aerospace Center (DLR) and others.
// This program and the accompanying materials
// are made available under the terms of the Eclipse Public License v2.0
// which accompanies this distribution, and is available at
// http://www.eclipse.org/legal/epl-v20.html
// SPDX-License-Identifier: EPL-2.0
/****************************************************************************/
/// @file    NIVissimSingleTypeParser_Verbindungsdefinition.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Wed, 18 Dec 2002
/// @version $Id$
///
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <iostream>
#include <utils/geom/PositionVector.h>
#include <utils/common/TplConvert.h>
#include "../NIImporter_Vissim.h"
#include "../tempstructs/NIVissimConnection.h"
#include "NIVissimSingleTypeParser_Verbindungsdefinition.h"


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimSingleTypeParser_Verbindungsdefinition::NIVissimSingleTypeParser_Verbindungsdefinition(NIImporter_Vissim& parent)
    : NIImporter_Vissim::VissimSingleTypeParser(parent) {}


NIVissimSingleTypeParser_Verbindungsdefinition::~NIVissimSingleTypeParser_Verbindungsdefinition() {}


bool
NIVissimSingleTypeParser_Verbindungsdefinition::parse(std::istream& from) {
    int id;
    from >> id; // type-checking is missing!
    std::string tag;
    // Read optional value "Name", skip optional value "Beschriftung"
    std::string name;
    while (tag != "von") {
        tag = overrideOptionalLabel(from);
        if (tag == "name") {
            name = readName(from);
        }
    }
    // Read the geometry information
    NIVissimExtendedEdgePoint from_def = readExtEdgePointDef(from);
    PositionVector geom;
    tag = myRead(from); // "ueber"
    while (tag != "nach") {
        std::string x = myRead(from);
        std::string y = myRead(from);
        if (y != "nach") {
            geom.push_back_noDoublePos(
                Position(
                    TplConvert::_2double(x.c_str()),
                    TplConvert::_2double(y.c_str())
                ));
            tag = myRead(from);
            try {
                TplConvert::_2double(tag.c_str());
                tag = myRead(from);
            } catch (NumberFormatException&) {}
        } else {
            tag = y;
        }
    }
    NIVissimExtendedEdgePoint to_def = readExtEdgePointDef(from);
    // read some optional values until mandatory "Fahrzeugklassen" occurs
    double dxnothalt = 0;
    double dxeinordnen = 0;
    double zuschlag1, zuschlag2;
    zuschlag1 = zuschlag2 = 0;
    double seglength = 0;
    tag = myRead(from);
//    NIVissimConnection::Direction direction = NIVissimConnection::NIVC_DIR_ALL;
    while (tag != "fahrzeugklassen" && tag != "sperrung" && tag != "auswertung" && tag != "DATAEND") {
        if (tag == "rechts") {
//            direction = NIVissimConnection::NIVC_DIR_RIGHT;
        } else if (tag == "links") {
//            direction = NIVissimConnection::NIVC_DIR_LEFT;
        } else if (tag == "alle") {
//            direction = NIVissimConnection::NIVC_DIR_ALL;
        } else if (tag == "dxnothalt") {
            from >> dxnothalt; // type-checking is missing!
        } else if (tag == "dxeinordnen") {
            from >> dxeinordnen; // type-checking is missing!
        } else if (tag == "segment") {
            from >> tag;
            from >> seglength;
        }
        if (tag == "zuschlag") {
            from >> zuschlag1; // type-checking is missing!
            tag = readEndSecure(from);
            if (tag == "zuschlag") {
                from >> zuschlag2; // type-checking is missing!
                tag = readEndSecure(from, "auswertung");
            }
        } else {
            tag = readEndSecure(from, "auswertung");
        }
    }
    // read in allowed vehicle classes
    std::vector<int> assignedVehicles;
    if (tag == "fahrzeugklassen") {
        tag = readEndSecure(from);
        while (tag != "DATAEND" && tag != "sperrung" && tag != "auswertung") {
            int classes = TplConvert::_2int(tag.c_str());
            assignedVehicles.push_back(classes);
            tag = readEndSecure(from, "auswertung");
        }
    }
    // Read definitions of closed lanes
    NIVissimClosedLanesVector clv;
    if (tag != "DATAEND") {
        do {
            // check whether a next close lane definition can be found
            tag = readEndSecure(from);
            if (tag == "keinspurwechsel") {
                while (tag != "DATAEND") {
                    tag = readEndSecure(from);
                }
            } else if (tag == "spur") {
                // get the lane number
//                from >> tag;
                int laneNo;
                from >> laneNo; // unused and type-checking is missing!
                // get the list of assigned car classes
                std::vector<int> assignedVehicles;
                tag = myRead(from);
                if (tag == "fahrzeugklassen") {
                    tag = myRead(from);
                }
                while (tag != "DATAEND" && tag != "spur" && tag != "keinspurwechsel") {
                    int classes = TplConvert::_2int(tag.c_str());
                    assignedVehicles.push_back(classes);
                    tag = readEndSecure(from);
                }
                // build and add the definition
                NIVissimClosedLaneDef* cld = new NIVissimClosedLaneDef(assignedVehicles);
                clv.push_back(cld);
            }
        } while (tag != "DATAEND");
    }
    NIVissimConnection* c = new NIVissimConnection(id, name, from_def, to_def, geom,
            assignedVehicles, clv);

    if (!NIVissimConnection::dictionary(id, c)) {
        return false;
    }
    return true;
    //return NIVissimAbstractEdge::dictionary(id, c);
}



/****************************************************************************/
