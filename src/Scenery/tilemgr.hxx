// tilemgr.hxx -- routines to handle dynamic management of scenery tiles
//
// Written by Curtis Olson, started January 1998.
//
// Copyright (C) 1997  Curtis L. Olson  - http://www.flightgear.org/~curt
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
// $Id$


#ifndef _TILEMGR_HXX
#define _TILEMGR_HXX

#include <simgear/compiler.h>

#include <simgear/bucket/newbucket.hxx>
#include "SceneryPager.hxx"
#include "tilecache.hxx"

namespace osg
{
class Node;
}

namespace simgear
{
class SGReaderWriterOptions;
}

class FGTileMgr {

private:

    // Tile loading state
    enum load_state {
        Start = 0,
        Inited = 1,
        Running = 2
    };

    load_state state, last_state;

    // schedule a tile for loading, returns true when tile is already loaded
    bool sched_tile( const SGBucket& b, double priority,bool current_view, double request_time);

    // schedule a needed buckets for loading
    void schedule_needed(const SGBucket& curr_bucket, double rangeM);

    bool isTileDirSyncing(const std::string& tileFileName) const;
    
    SGBucket previous_bucket;
    SGBucket current_bucket;
    SGBucket pending;
    osg::ref_ptr<simgear::SGReaderWriterOptions> _options;

    double scheduled_visibility;

    /**
     * tile cache
     */
    TileCache tile_cache;
    
    class TileManagerListener;
    friend class TileManagerListener;
    std::unique_ptr<TileManagerListener> _listener;

    // update various queues internal queues
    void update_queues(bool& isDownloadingScenery);

    // schedule tiles for the viewer bucket
    void schedule_tiles_at(const SGGeod& location, double rangeM);

    SGPropertyNode_ptr _visibilityMeters;
    SGPropertyNode_ptr _lodDetailed, _lodRoughDelta, _lodBareDelta, _disableNasalHooks;
    SGPropertyNode_ptr _scenery_loaded, _scenery_override;

    osg::ref_ptr<flightgear::SceneryPager> _pager;

    /// is caching of expired tiles enabled or not?
    bool _enableCache;    
    bool _use_vpb;
public:
    FGTileMgr();
    ~FGTileMgr();

    // Initialize the Tile Manager
    void init();
    void reinit();
    void shutdown();
    void update(double dt);

    const SGBucket& get_current_bucket () const { return current_bucket; }

    // Returns true if scenery is available for the given lat, lon position
    // within a range of range_m.
    // lat and lon are expected to be in degrees.
    bool schedule_scenery(const SGGeod& position, double range_m, double duration=0.0);

    // Returns true if tiles around current view position have been loaded
    bool isSceneryLoaded();
    
    // notify the tile manahger the material library was reloaded,
    // so it can pass this through to its options object
    void materialLibChanged();
};

#endif // _TILEMGR_HXX
