// component.hxx - Base class for autopilot components
//
// Written by Torsten Dreyer
// Based heavily on work created by Curtis Olson, started January 2004.
//
// Copyright (C) 2004  Curtis L. Olson  - http://www.flightgear.org/~curt
// Copyright (C) 2010  Torsten Dreyer - Torsten (at) t3r (dot) de
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
//
#pragma once

#include <simgear/structure/subsystem_mgr.hxx>
#include <simgear/props/propsfwd.hxx>

namespace FGXMLAutopilot {

/**
 * @brief Base class for other autopilot components
 */
class Component : public SGSubsystem
{
private:
    SGSharedPtr<const SGCondition> _condition;
    SGPropertyNode_ptr _enable_prop;
    std::string * _enable_value;
    bool _enabled;

protected:
    virtual bool configure( SGPropertyNode& cfg_node,
                            const std::string& cfg_name,
                            SGPropertyNode& prop_root );

   /**
    * @brief pure virtual function to be implemented by the derived classes. Gets called from
    * the update method if it's not disabled with the firstTime parameter set to true if this
    * is the first call after being enabled
    * @param firstTime set to true if this is the first update call since this component has
             been enabled. Set to false for every subsequent call.
    * @param dt  the elapsed time since the last call
    */
    virtual void update( bool firstTime, double dt ) = 0;

   /**
    * @brief overideable method being called from the update() method if this component
    *        is disabled. It's a noop by default.
    */
    virtual void disabled( double dt ) {}

    /**
     * @brief debug flag, true if this component should generate some useful output
     * on every iteration
     */
    bool _debug;


    /**
     * @brief a (historic) flag signalling the derived class that it should compute it's internal
     *        state but shall not set the output properties if /autopilot/locks/passive-mode is true.
     */
    bool _honor_passive;

public:
    /**
     * @brief A constructor for an empty Component.
     */
    Component();

    /**
     * virtual destructor to clean up resources
     */
    virtual ~Component();

    // Subsystem API.
    void update(double dt) override;

    /**
     * @brief configure this component from a property node. Iterates through
     *        all nodes found as children under configNode and calls configure
     *        of the derived class for each child.
     *
     * @param prop_root Property root for all relative paths
     * @param cfg       Property node containing the configuration
     */
    virtual bool configure( SGPropertyNode& prop_root,
                            SGPropertyNode& cfg );

    /**
     * @brief check if this component is enabled as configured in the
     * &lt;enable&gt; section
     * @return true if the enable-condition is true.
     *
     * If a &lt;condition&gt; is defined, this condition is evaluated,
     * &lt;prop&gt; and &lt;value&gt; tags are ignored.
     *
     * If a &lt;prop&gt; is defined and no &lt;value&gt; is defined, the property
     * named in the &lt;prop&gt;&lt;prop&gt; tags is evaluated as boolean.
     *
     * If a &lt;prop&gt; is defined and a &lt;value&gt; is defined, the property named
     * in &lt;prop&gt;&lt;/prop&gt; is compared (as a string) to the value defined in
     * &lt;value&gt;&lt;/value&gt;
     *
     * Returns true, if neither &lt;condition&gt; nor &lt;prop&gt; exists
     */
    bool isPropertyEnabled();
};

} // of namespace FGXMLAutopilot

