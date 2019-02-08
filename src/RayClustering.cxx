#include "WireCellUtil/RayClustering.h"

using namespace WireCell;
using namespace WireCell::RayGrid;

WireCell::RayGrid::blobproj_t WireCell::RayGrid::projection(const blobvec_t& blobs, layer_index_t layer)
{
    blobproj_t ret;
    for (blobref_t blob : blobs) {
        const auto bounds = blob->strips()[layer].bounds;
        for (auto gi = bounds.first; gi != bounds.second; ++gi) {
            if ((int)ret.size() <= gi) {
                ret.resize(gi+1);
            }
            ret[gi].push_back(blob);
        }
    }
    return ret;
}


WireCell::RayGrid::blobvec_t WireCell::RayGrid::select(const blobproj_t& proj, grid_range_t gr)
{
    blobset_t uniq;
    for (grid_index_t gind = gr.first; gind < gr.second; ++gind) {
        const auto& some = proj[gind];
        uniq.insert(some.begin(), some.end());
    }
    return blobvec_t(uniq.begin(), uniq.end());
}


WireCell::RayGrid::blobvec_t WireCell::RayGrid::overlap(const blobref_t& blob, const blobproj_t& proj, layer_index_t layer)
{
    const auto& strip = blob->strips()[layer];
    auto blobs = select(proj, strip.bounds);
    if (layer == 0) {
        return blobs;
    }
    --layer;
    auto newproj = projection(blobs, layer);
    return overlap(blob, newproj, layer);
}


WireCell::RayGrid::blobvec_t WireCell::RayGrid::references(const blobs_t& blobs)
{
    const size_t siz = blobs.size();
    blobvec_t ret(siz);
    for (size_t ind=0; ind<siz; ++ind) {
        ret[ind] = blobs.begin()+ind;
    }
    return ret;
}

void WireCell::RayGrid::associate(const blobs_t& one, const blobs_t& two, associator_t func)
{
    const size_t nlayers = two[0].strips().size();
    const size_t ilayer = nlayers-1;
    const auto proj = projection(references(two), ilayer);
    for (blobref_t blob = one.begin(); blob != one.end(); ++blob) {
        auto assoc = overlap(blob, proj, ilayer);
        for (blobref_t other : assoc) {
            func(blob, other);
        }
    }
}



/*
for blob in slice[i]:

  other_blobs = slice[i+1];
  for strip in blob.strips:
    other_blobs = overlapping_blobs(other_blobs, strip)

  if other_blobs.empty():
    continue

  mark(blob)
  for other in other_blobs:
    mark(other)
*/
