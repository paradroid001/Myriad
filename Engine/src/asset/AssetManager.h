/*
- At Design Time
  - Specify where to find assets
  - Create GUIDs for assets, use hashes so you can detect moves, and keep track
of the current path.
  - allow 'composite' assets that reference other assets - i.e. a heirarchy
  - different assets have different metadata
  - metadata, guids, etc need to be serialised
  - Allow changing / editing of asset metadata/settings
  - Allow 'freezing' out what actual assets exist in the game (don't build
things you don't need to)
  - Allow zipping up or packing of resources (doing any required processing)
  - Stretch: a res manager GUI

- At Run Time
  - Loading resources async / streaming / on demand
  - Manage lifetime of resources
    - Destruction of resources when no longer needed
  - Ensuring only one copy of a resource is loaded
  - Handles 'composite' or parented resoueces.
  - Allow any init processing
*/
