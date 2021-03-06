// SwiftAircraftManager.cpp - Manager class for aircraft generated by swift
//
// Copyright (C) 2019 - swift Project Community / Contributors (http://swift-project.org/)
// Adapted to Flightgear by Lars Toenning <dev@ltoenning.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "SwiftAircraftManager.h"
#include <Main/globals.hxx>
#include <utility>

FGSwiftAircraftManager::FGSwiftAircraftManager()
{
    m_initialized = true;
}

FGSwiftAircraftManager::~FGSwiftAircraftManager()
{
    this->removeAllPlanes();
    m_initialized = false;
}

bool FGSwiftAircraftManager::isInitialized() const
{
    return m_initialized;
}

bool FGSwiftAircraftManager::addPlane(const std::string& callsign, const std::string& modelString)
{
    this->removePlane(callsign); // Remove plane if already exists e.g. when rematching is done.
    auto curAircraft = FGAISwiftAircraftPtr(new FGAISwiftAircraft(callsign, modelString));
    globals->get_subsystem<FGAIManager>()->attach(curAircraft);
    // Init props after prop-root is assigned
    curAircraft->initProps();

    aircraftByCallsign.insert(std::pair<std::string, FGAISwiftAircraftPtr>(callsign, curAircraft));
    return true;
}

void FGSwiftAircraftManager::updatePlanes(std::vector<std::string> callsigns, std::vector<SGGeod> positions, std::vector<SGVec3d> orientations, std::vector<double> groundspeeds, std::vector<bool> onGrounds)
{

	for (long unsigned int i = 0; i < callsigns.size(); i++)
    {
        auto it = aircraftByCallsign.find(callsigns.at(i));
        if(it != aircraftByCallsign.end())
        {
            it->second->updatePosition(positions.at(i), orientations.at(i), groundspeeds.at(i),true);
        }
    }
}

void FGSwiftAircraftManager::getRemoteAircraftData(std::vector<std::string>& callsigns, std::vector<double>& latitudesDeg, std::vector<double>& longitudesDeg, std::vector<double>& elevationsM, std::vector<double>& verticalOffsets) const
{
    if (callsigns.empty() || aircraftByCallsign.empty()) { return; }

    const auto requestedCallsigns = callsigns;
    callsigns.clear();
    latitudesDeg.clear();
    longitudesDeg.clear();
    elevationsM.clear();
    verticalOffsets.clear();

	for (const auto & requestedCallsign : requestedCallsigns) {
        const auto it = aircraftByCallsign.find(requestedCallsign);
        if(it == aircraftByCallsign.end()) { continue; }

        const FGAISwiftAircraft* aircraft = it->second;
        assert(aircraft);


        SGGeod pos;
        pos.setLatitudeDeg(aircraft->_getLatitude());
        pos.setLongitudeDeg(aircraft->_getLongitude());
        const double latDeg = pos.getLatitudeDeg();
        const double lonDeg = pos.getLongitudeDeg();
        double groundElevation = aircraft->getGroundElevation(pos);

        callsigns.push_back(requestedCallsign);
        latitudesDeg.push_back(latDeg);
        longitudesDeg.push_back(lonDeg);
        elevationsM.push_back(groundElevation);
        verticalOffsets.push_back(0);
	}
}

void FGSwiftAircraftManager::removePlane(const std::string& callsign)
{
    auto it = aircraftByCallsign.find(callsign);
    if(it != aircraftByCallsign.end())
    {
        it->second->setDie(true);
        aircraftByCallsign.erase(it);
    }
}

void FGSwiftAircraftManager::removeAllPlanes()
{
    for(auto it = aircraftByCallsign.begin(); it!= aircraftByCallsign.end();)
    {
        it->second->setDie(true);
        it = aircraftByCallsign.erase(it);
    }
}

double FGSwiftAircraftManager::getElevationAtPosition(const std::string &callsign, const SGGeod& pos) const
{
    auto it = aircraftByCallsign.find(callsign);
    if(it != aircraftByCallsign.end())
    {
        return it->second->getGroundElevation(pos);
    }
    // Aircraft not found in list
    return std::numeric_limits<double>::quiet_NaN();
}

void FGSwiftAircraftManager::setPlanesTransponders(const std::vector<AircraftTransponder>& transponders)
{
    for (const auto & transponder : transponders)
    {
        auto it = aircraftByCallsign.find(transponder.callsign);
        if(it != aircraftByCallsign.end())
        {
            it->second->setPlaneTransponder(transponder);
        }
    }
}

void FGSwiftAircraftManager::setPlanesSurfaces(const std::vector<AircraftSurfaces>& surfaces)
{
    for (const auto & surface : surfaces)
    {
        auto it = aircraftByCallsign.find(surface.callsign);
        if(it != aircraftByCallsign.end())
        {
            it->second->setPlaneSurface(surface);
        }
    }
}
