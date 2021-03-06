// TileCache.cxx -- routines to handle scenery tile caching
//
// Written by Curtis Olson, started December 2000.
//
// Copyright (C) 2000  Curtis L. Olson  - http://www.flightgear.org/~curt
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <simgear/bucket/newbucket.hxx>
#include <simgear/debug/logstream.hxx>
#include <simgear/misc/sg_path.hxx>

#include "tileentry.hxx"
#include "tilecache.hxx"

TileCache::TileCache( void ) :
    max_cache_size(100), current_time(0.0)
{
    tile_cache.clear();
}


TileCache::~TileCache( void )
{
    tile_map_iterator it = tile_cache.begin();
    for (; it != tile_cache.end(); ++it) {
        TileEntry* tile = it->second;
        tile->removeFromSceneGraph();
        delete tile;
    }
}


// Free a tile cache entry
void TileCache::entry_free( long tile_index ) {
    SG_LOG( SG_TERRAIN, SG_DEBUG, "FREEING CACHE ENTRY = " << tile_index );
    TileEntry *tile = tile_cache[tile_index];
    tile->removeFromSceneGraph();
    tile_cache.erase( tile_index );
    delete tile;
}


// Initialize the tile cache subsystem
void TileCache::init( void ) {
    SG_LOG( SG_TERRAIN, SG_INFO, "Initializing the tile cache." );

    SG_LOG( SG_TERRAIN, SG_INFO, "  max cache size = "
            << max_cache_size );
    SG_LOG( SG_TERRAIN, SG_INFO, "  current cache size = "
            << tile_cache.size() );

    clear_cache();

    SG_LOG( SG_TERRAIN, SG_INFO, "  done with init()"  );
}


// Search for the specified "bucket" in the cache
bool TileCache::exists_stg( const SGBucket& b ) const {
    long tile_index = b.gen_index();
    const_tile_map_iterator it = tile_cache.find( tile_index );

    return ( it != tile_cache.end() );
}

bool TileCache::exists_vpb( const SGBucket& b ) const {
    // VPB tiles are stored with negative index to avoid clash with STG index
    long tile_index =  - b.gen_vpb_index();
    const_tile_map_iterator it = tile_cache.find( tile_index );

    return ( it != tile_cache.end() );
}


// Return the index of a tile to be dropped from the cache, return -1 if
// nothing available to be removed.
long TileCache::get_drop_tile() {
    long min_index = -1;
    double min_time = DBL_MAX;
    float priority = FLT_MAX;

    tile_map_iterator current = tile_cache.begin();
    tile_map_iterator end = tile_cache.end();

    for ( ; current != end; ++current ) {
        long index = current->first;
        TileEntry *e = current->second;
        if (( !e->is_current_view() )&&
            ( e->is_expired(current_time) ))
        {
            if (e->is_expired(current_time - 1.0)&&
                !e->is_loaded())
            {
                /* Immediately drop "empty" tiles which are no longer used/requested, and were last requested > 1 second ago...
                 * Allow a 1 second timeout since an empty tiles may just be loaded...
                 */
                SG_LOG( SG_TERRAIN, SG_DEBUG, "    dropping an unused and empty tile");
                min_index = index;
                break;
            }
            if (( e->get_time_expired() < min_time )||
                (( e->get_time_expired() == min_time)&&
                 ( priority > e->get_priority())))
            {
                // drop oldest tile with lowest priority
                min_time = e->get_time_expired();
                priority = e->get_priority();
                min_index = index;
            }
        }
    }

    SG_LOG( SG_TERRAIN, SG_DEBUG, "    index = " << min_index );
    SG_LOG( SG_TERRAIN, SG_DEBUG, "    min_time = " << min_time );

    return min_index;
}

long TileCache::get_first_expired_tile() const
{
  const_tile_map_iterator current = tile_cache.begin();
  const_tile_map_iterator end = tile_cache.end();

  for ( ; current != end; ++current ) {
    TileEntry *e = current->second;
    if (!e->is_current_view() && e->is_expired(current_time))
    {
      return current->first;
    }
  }

  return -1; // no expired tile found
}


// Clear all flags indicating tiles belonging to the current view
void TileCache::clear_current_view()
{
    tile_map_iterator current = tile_cache.begin();
    tile_map_iterator end = tile_cache.end();

    for ( ; current != end; ++current ) {
        TileEntry *e = current->second;
        if (e->is_current_view())
        {
            // update expiry time for tiles belonging to most recent position
            e->update_time_expired( current_time );
            e->set_current_view( false );
        }
    }
}

// Clear a cache entry, note that the cache only holds pointers
// and this does not free the object which is pointed to.
void TileCache::clear_entry( long tile_index ) {
    tile_cache.erase( tile_index );
}


// Clear all completely loaded tiles (ignores partially loaded tiles)
void TileCache::clear_cache() {
    std::vector<long> indexList;
    tile_map_iterator current = tile_cache.begin();
    tile_map_iterator end = tile_cache.end();

    for ( ; current != end; ++current ) {
        long index = current->first;
        TileEntry *e = current->second;
        if ( e->is_loaded() ) {
            e->tile_bucket.make_bad();
            // entry_free modifies tile_cache, so store index and call entry_free() later;
            indexList.push_back( index);
        }
    }
    for (unsigned int it = 0; it < indexList.size(); it++) {
        entry_free( indexList[ it]);
    }
}

/**
 * Create a new tile and schedule it for loading.
 */
bool TileCache::insert_tile( STGTileEntry *e ) {
    // register tile in the cache
    long tile_index = e->get_tile_bucket().gen_index();
    tile_cache[tile_index] = e;
    e->update_time_expired(current_time);

    return true;
}

/**
 * Create a new tile and schedule it for loading.  VPB version, with negative index.
 */
bool TileCache::insert_tile( VPBTileEntry *e ) {
    // register tile in the cache
    long tile_index = - e->get_tile_bucket().gen_vpb_index();
    tile_cache[tile_index] = e;
    e->update_time_expired(current_time);

    return true;
}


// update tile's priority and expiry time according to current request
void TileCache::request_tile(TileEntry* t,float priority,bool current_view,double request_time)
{
    if ((!current_view)&&(request_time<=0.0))
        return;

    // update priority when higher - or old request has expired
    if ((t->is_expired(current_time))||
         (priority > t->get_priority()))
    {
        t->set_priority( priority );
    }

    if (current_view)
    {
        t->update_time_expired( current_time + request_time );
        t->set_current_view( true );
    }
    else
    {
        t->update_time_expired( current_time+request_time );
    }
}

// Return a pointer to the specified tile cache entry
STGTileEntry* TileCache::get_stg_tile( const SGBucket& b ) const {

    const_tile_map_iterator it = std::find_if(tile_cache.begin(), tile_cache.end(), 
                        [b](auto &t) {
                            return ((b.gen_index() == t.first) && (t.second->getExtension() == TileEntry::Extension::STG));
                        });
    if ( it != tile_cache.end() ) {
        return dynamic_cast<STGTileEntry*>(it->second);
    } else {
        return NULL;
    }
}

// Return a pointer to the specified tile cache entry
VPBTileEntry* TileCache::get_vpb_tile( const SGBucket& b ) const {
    const_tile_map_iterator it = std::find_if(tile_cache.begin(), tile_cache.end(), 
                        [b](auto &t) {
                            // Negative indices are used for the VPB tiles.
                            return (( - b.gen_vpb_index() == t.first) && (t.second->getExtension() == TileEntry::Extension::VPB));
                        });
    if ( it != tile_cache.end() ) {
        return dynamic_cast<VPBTileEntry*>(it->second);
    } else {
        return NULL;
    }
}
