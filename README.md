# TIMELOCK : team-03

## M3 Required Elements:
<table>
  <tr>
   <td><strong># / Task</strong>
   </td>
   <td><strong>Category</strong>
   </td>
   <td><strong>Implemented Where?</strong>
   </td>
   <td><strong>Notes</strong>
   </td>
  </tr>
  <tr>
   <td>[1] 5-minutes of unique gameplay
   </td>
   <td>Playability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>[2] Memory management
   </td>
   <td>Robustness
   </td>
   <td><code>World System</code> <code>ECS</code>
   </td>
   <td>Global list to cache pointers, smart pointers, registry
   </td>
  </tr>
  <tr>
   <td>[3] User Input 
   </td>
   <td>Robustness
   </td>
   <td><code>World System</code>
   </td>
   <td>Currently no actual invalid input available from the user.
   </td>
  </tr>
  <tr>
   <td>[4] Mesh Based Collision
   </td>
   <td>Robustness
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>[5] Stability
   </td>
   <td>Stability 
   </td>
   <td><code>Whole application</code>
   </td>
   <td>Levels are parsed from JSON. Text is rendered to the screen as sprites.
   </td>
  </tr>
  <tr>
   <td>[6] Test plan - a list of player or game actions and their expected outcomes.
   </td>
   <td>Software Engineering
   </td>
   <td>
   </td>
   <td>
<code>https://docs.google.com/document/d/1RFVZJj_J0ug45Zf6cPg0DK9aR9maLVpcoV6uE7CV40g/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[7] Bug List
   </td>
   <td>Reporting
   </td>
   <td>
   </td>
   <td>
<code>https://docs.google.com/spreadsheets/d/14reigVHCiUrnIVMnTQdUnz6nX_JFBuTTT_pZXGZ06WA/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[8] Video
   </td>
   <td>Reporting
   </td>
   <td>
   </td>
   <td>
<code>https://www.youtube.com/watch?v=p3mYLYqT8v8</code>
   </td>
  </tr>
</table>



# M3 Creative Elements

** these are the creative elements we wish to have graded for this milestone

<table>
  <tr>
   <td><strong>Element</strong>
   </td>
   <td><strong>Category</strong>
   </td>
   <td><strong>Level</strong>
   </td>
   <td><strong>Implemented Where? </strong>
   </td>
   <td><strong>Description</strong>
   </td>
  </tr>
  <tr>
   <td>[5] Particle system
   </td>
   <td>Graphics
   </td>
   <td>Advanced
   </td>
   <td><code>Particle_system.hpp</code> <code>rendering_system_utils.cpp</code>
   </td>
   <td>Particle system supported by instanced rendering   </td>
  </tr>
  <tr>
   <td>[13] Physics based Animations
   </td>
   <td>Physics & Simulation 
   </td>
   <td>Advanced
   </td>
   <td><code>physics_simulation.cpp</code>
   </td>
   <td>Impulse based physics for collisions + simple gravity pendulums.
   </td>
  </tr>
</table>



## Other Notes:
- Background music source: https://www.youtube.com/watch?v=gfnQIaFBKIE
- Slowdown & speedup sound effect source: https://www.youtube.com/watch?v=z_epWpdsHJc
- How to run on linux/mac (and compile with debug symbols):
  - Navigate to `ECS_DRAFT/build`.
  - Run `cmake ..` (or `cmake -DCMAKE_BUILD_TYPE=Debug ..` for debug symbols)
  - Run `make` to compile changes
  - Run `./TIMELOCK`

- How to build on Windows Visual Studio:
  - Open folder `ECS_DRAFT`, wait for CMakeList.txt file to be loaded
  - Can also manually configure CMake cache by going to `Navigation bar` -> `Project` -> `Configure Cache`
  - Update CMake settings according to need; we suggest build and run release version of the game
    - Windows user can go to `/ECS_DRAFT/CMakeSettings.json` and add build modes, such as `x64-Release`
  - Build `TIMELOCK.exe`
  - Run `TIMELOCK.exe`
 
- How to use co author script:
    - make sure it is executable (`chmod +x ./coAuthorCommit.sh`)
    - Add all the files to git (`git add -p`)
    - Run the commit script to make the commit `./coAuthorCommit.sh "NAME" "EMAIL" "COMMIT MESSAGE"`
    - Push (`git push`)
